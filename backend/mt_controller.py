import os
import sys
import re
import numpy as np
import base64
import pandas as pd

from imageio import imread
from imageio import imsave
from skimage.transform import resize

sys.path.append('./meters')
from meters.batch import MeterBatch
from meters.dataset import FilesIndex, Dataset, Pipeline, V, B
from meters.dataset.dataset.models.tf import TFModel

# sys.path.append('./Project')
from face_batch import CelebrityBatch, load_func
from dataset import FilesIndex, Dataset, Pipeline, V, B

sys.path.append('./Project')
from model import MyModel
#from dataset import FilesIndex, Dataset, Pipeline, V, B

sys.path.append("./default_meters/")
sys.path.append("./uploaded_meters/")

class MtController:
    def __init__(self):
        print('entered init')
	
        model_path = os.path.join(os.getcwd(), 'Project/model/')
        model_name =  'model-20170512-110547'
        checkpoint_path = 'model-20170512-110547.ckpt-250000'
        index_path = 'Project/150k_50t_index.ann'
        int_mapping_file = 'Project/int_indices_mapping.csv'

        IMG_SHAPE = (160, 160)
        K_NEIGHBOURS = 15
	
        self.src_images = 'cropped_photos/'
        self.meters_path = os.path.join(os.getcwd(), "default_meters")
        self.uploaded_files_path = os.path.join(os.getcwd(), "uploaded_meters")
        self.meters_filenames = sorted(os.listdir(self.meters_path))
        self.output_shape = (500, 500)

        self.find_neighbours_ppl = (Pipeline()
                                         .load(fmt='image', components='images')
                                         .to_array()
                                         .to_cv(src='images')
                                         .detect_face()
                                         .crop_from_bbox()
                                         .resize(IMG_SHAPE, fmt='cv')
                                         .to_rgb()
                                         .init_variable('predicted_embeddings', init_on_each_run=0)
                                         .init_variable('indices', init_on_each_run=0)
                                         .init_model('static', MyModel, model_name,
                                                     config={'load' : {'path' : model_path, 'graph': model_name + '.meta',
                                                             'checkpoint' : checkpoint_path},  'build': False})
                                         .predict_model(model_name, fetches="embeddings:0",
                                                        feed_dict={'input:0' : B('images'), 'phase_train:0' : False},
                                                        save_to=B('embedding'), mode='w')
                                         .find_nearest_neighbours(src=index_path, k_neighbours=K_NEIGHBOURS)
                                   )
        self.indices_mapping = pd.read_csv(int_mapping_file, names=['file_name', 'int_index'], index_col='int_index')

    def build_ds(self, path):
        print('BUILDING DATASET')
        return Dataset(index=FilesIndex(path=path), batch_class=CelebrityBatch)

    def get_list(self, data, meta):
        print("DEFAULT LIST CONTAINS " + str(len(self.meters_filenames)) + ' ITEMS')
        return dict(data=[dict(id='default/' + fname) for fname in self.meters_filenames],
                    meta=meta)

    def _read_image(self, image_name):
        print(os.path.join(self.meters_path, image_name))
        return imread(os.path.join(self.meters_path, image_name))

    def get_item_data(self, data, meta):
        print('GET_ITEM_DATA CALLED')
        image_type, image_name = data['id'].split('/')
        if image_type != "default":
            print("Error: wrong image type. Expected \"default\", got {0}".format(image_type))
            return dict(data=data, meta=meta)
        image = self._read_image(image_name)
        path = os.path.join(self.meters_path, image_name)
        print('-' * 10 + 'reading' + path)
        with open(path, 'rb') as f:
            img = base64.b64encode(bytearray(f.read()))
        print('-' * 10 + 'success')
        data['src'] = img
        return dict(data=data, meta=meta)

    def upload_image(self, data, meta):
        print("UPLOAD IMAGE CALLED")
        image_data = data['src']
        image_type, image_name = data['id'].split('/')
        if image_type != "uploaded":
            print("Error: wrong image type. Expected \"uploaded\", got {0}".format(image_type))
            return dict(data=data, meta=meta)
        path = os.path.join(self.uploaded_files_path, image_name)
        print('-' * 10 + 'writing' + path)
        with open(path, 'wb') as f:
            f.write(base64.b64decode(image_data.split('base64,')[1]))
        print('-' * 10 + 'success')    
        data = {'id': data['id']}      
        return self.get_inference(data, meta)

    def get_inference(self, data, meta):
        print('GET_INFERENCE CALLED')
        item_type, item_name = data['id'].split('/')
        if item_type == "default":
            path = os.path.join(self.meters_path, item_name)
        elif item_type == "uploaded":
            path = os.path.join(self.uploaded_files_path, item_name)
        else:
            print("Unknown item type", item_type)
            return dict(data=data, meta=meta)
        image = imread(path)
        self.output_shape = image.shape[:2]
        
        dset = self.build_ds(path)
        print('dataset has been built', dset.indices)
        pred = self.find_neighbours_ppl << dset
        print('created pred')
        batch = pred.next_batch(1)
        print('got next batch')
        print('image.shape', image.shape[1::-1])
        knn = [self.indices_mapping.loc[neighbour_index, 'file_name'] for neighbour_index in batch.neighbours[0][:1]]
        knn_data = []
        for current in knn:
            img_file = os.path.join(self.src_images, str(current) + '.jpg')
            image_height, image_width = imread(img_file).shape[:2]
            with open(img_file, 'rb') as f:
                img = base64.b64encode(bytearray(f.read()))
            knn_data.append({'src': img, 'height': image_height, 'width': image_width})

        # bbox = pred.get_variable('bbox_predictions')[0]
        # * np.tile(self.output_shape, 2) / np.tile(image.shape[1::-1], 2)
        # labels = pred.get_variable('labels')
       
        bbox=np.array([0.0, 10, 50, 50])
        
        inference = {"bbox": bbox.tolist(), "knn_data": knn_data, "value": ''.join(map(str, knn))}
        
        data["inference"] = inference
        print('inference data', data)
        return dict(data=data, meta=meta)
