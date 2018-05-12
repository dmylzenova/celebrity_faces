import os
import pandas as pd

from dataset import FilesIndex, Dataset, Pipeline, B

from face_batch import CelebrityBatch, load_func
from model import MyModel


class MtController:
    def __init__(self, k_neighbours=15, img_shape=(160, 160),
                 cropped_photos_dir='cropped_photos',
                 index_path='data/150k_50t_index.ann',
                 model_name='model-20170512-110547',
                 checkpoint_path='model-20170512-110547.ckpt-250000',
                 int_mapping_file='data/int_indices_mapping.csv',
                 model_path=os.path.join(os.getcwd(), 'model/')):

        self.cropped_photos_dir = cropped_photos_dir

        self.find_neighbours_ppl = (Pipeline()
                                         .load(fmt='image', components='images')
                                         .to_array()
                                         .to_cv(src='images')
                                         .detect_face()
                                         .crop_from_bbox()
                                         .resize(img_shape, fmt='cv')
                                         .to_rgb()
                                         .init_variable('predicted_embeddings', init_on_each_run=0)
                                         .init_variable('indices', init_on_each_run=0)
                                         .init_model('static', MyModel, model_name,
                                                     config={'load': {'path': model_path, 'graph': model_name + '.meta',
                                                             'checkpoint': checkpoint_path},  'build': False})
                                         .predict_model(model_name, fetches="embeddings:0",
                                                        feed_dict={'input:0': B('images'), 'phase_train:0': False},
                                                        save_to=B('embedding'), mode='w')
                                         .find_nearest_neighbours(src=index_path, k_neighbours=k_neighbours)
                                   )
        self.indices_mapping = pd.read_csv(int_mapping_file, names=['file_name', 'int_index'], index_col='int_index')
        print('Initialized MtController...')

    def build_ds(self, path):
        print('BUILDING DATASET')
        return Dataset(index=FilesIndex(path=path), batch_class=CelebrityBatch)

    def get_inference(self, path):
        print('GET_INFERENCE CALLED')
        
        dset = self.build_ds(path)
        print('dataset has been built', dset.indices)
        pred = self.find_neighbours_ppl << dset
        print('created pred')
        batch = pred.next_batch(1)
        print('got next batch')
        knn = [self.indices_mapping.loc[neighbour_index, 'file_name'] for neighbour_index in batch.neighbours[0][:1]]
        knn_files = [os.path.join(self.cropped_photos_dir, str(current) + '.jpg') for current in knn]
        return knn_files
