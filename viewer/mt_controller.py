import os
import pandas as pd

import numpy as np
from dataset import FilesIndex, Dataset, Pipeline, B

import pylsh
import face_batch
import model


class MtController:
    def __init__(self, k_neighbours=15, img_shape=(160, 160),
                 model_path=os.path.dirname(__file__) + '/app/static/model',
                 model_name='model-20170512-110547',
                 checkpoint='model-20170512-110547.ckpt-250000',
                 cropped_photos_dir=os.path.dirname(__file__) + '/app/static/cropped_photos',
                 index_path=os.path.dirname(__file__) + '/app/static/data/',
                 haarcascade_xml_path=os.path.dirname(__file__) + '/app/static/haarcascade_frontalface_default.xml',
                 pylsh_params=(50, 64, 128)):
               
        self.cropped_photos_dir = cropped_photos_dir
        
        self.planes_path = (index_path + 'split.txt').encode(encoding='UTF-8')
        self.hash_tables_dir_path = (index_path + 'index/').encode(encoding='UTF-8')
        self.index_embedding_dict_path = (index_path + 'index_embedding.txt').encode(encoding='UTF-8')

        print('before lsh')
        self.index = pylsh.PyLSH(*pylsh_params)
        self.index.fill_data_from_files(planes_path=self.planes_path, hash_tables_dir_path=self.hash_tables_dir_path,
                                        index_embedding_dict_path=self.index_embedding_dict_path)
        print('initialized lsh')
        # test index
        if len(self.index.find_k_neighbors(5, np.random.normal(0, 1, 128))) == 0:
            print('EMPTY INDEX')
        else:
            print('successfully loaded LSH....')

        self.find_neighbours_ppl = (Pipeline()
                                         .load(fmt='image', components='images')
                                         .to_array()
                                         .to_cv(src='images')
                                         .detect_face(haarcascade_xml_path)
                                         .crop_from_bbox()
                                         .resize(img_shape, fmt='cv')
                                         .to_rgb()
                                         .init_variable('predicted_embeddings', init_on_each_run=0)
                                         .init_variable('indices', init_on_each_run=0)
                                         .init_model('static', model.MyModel, model_name,
                                                     config={'load': {'path': model_path, 'graph': model_name + '.meta',
                                                             'checkpoint': checkpoint}, 'build': False})
                                         .predict_model(model_name, fetches="embeddings:0",
                                                        feed_dict={'input:0': B('images'), 'phase_train:0': False},
                                                        save_to=B('embedding'), mode='w')
                                         .find_nearest_neighbours(self.index, k_neighbours=k_neighbours, use_pylsh=True)
                                   )

        print('Initialized MtController...')

    def build_ds(self, path):
        print('BUILDING DATASET')
        return Dataset(index=FilesIndex(path=path), batch_class=face_batch.CelebrityBatch)

    def get_inference(self, path, images_count=1):
        print('GET_INFERENCE CALLED')
        print('here is what i got ', path)
        dset = self.build_ds(path)
        print('dataset has been built', dset.indices)
        pred = self.find_neighbours_ppl << dset
        print('created pred')
        batch = pred.next_batch(1)
        print('got next batch')
        knn = [self.indices_mapping.loc[neighbour_index, 'file_name']
               for neighbour_index in batch.neighbours[0][:images_count]]
        knn_files = [str(current) + '.jpg' for current in knn]
        return knn_files
