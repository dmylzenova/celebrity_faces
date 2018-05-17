import os
import imageio

import pandas as pd
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
                 index_path=os.path.dirname(__file__) + '/app/static/index/',
                 haarcascade_xml_path=os.path.dirname(__file__) + '/app/static/haarcascade_frontalface_default.xml',
                 pylsh_params=(50, 64, 128)):
               
        self.cropped_photos_dir = cropped_photos_dir
        
        self.planes_path = (index_path + 'split.txt').encode(encoding='UTF-8')
        self.hash_tables_dir_path = (index_path + 'index/').encode(encoding='UTF-8')
        self.index_embedding_dict_path = (index_path + 'index_embedding.txt').encode(encoding='UTF-8')

        print('LSH init...')
        self.index = pylsh.PyLSH(*pylsh_params)
        assert self.index.fill_data_from_files(planes_path=self.planes_path,
                                               hash_tables_dir_path=self.hash_tables_dir_path,
                                               index_embedding_dict_path=self.index_embedding_dict_path)
        print('Successfully loaded LSH')

        embeddings_data = pd.read_table(self.index_embedding_dict_path.decode('UTF-8'), names=['embd'])
        embeddings_data['file_index'] = embeddings_data.embd.apply(lambda x: int(str(x)[:x.find(' ')]))
        embeddings_data['embeddings'] = embeddings_data.embd.apply(lambda x: list(map(float, x[x.find(' ') + 1:-1].split(' '))))
        self.file_indices = embeddings_data.file_index.values.tolist()
        self.saved_embeddings = embeddings_data.embeddings.values.tolist()
        print('Loaded embeddings')

        self.find_neighbours_ppl = (Pipeline()
                                         .load(fmt='image', components='images')
                                         .to_array()
                                         .to_cv(src='images')
                                         .detect_face(haarcascade_xml_path)
                                         .crop_from_bbox(margin=0.2, top_margin=2)
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
                                         .find_nearest_neighbours(pyindex=self.index, k_neighbours=k_neighbours, use_pylsh=True)
                                   )

        print('Ready!')

    def build_ds(self, path):
        print('BUILDING DATASET')
        return Dataset(index=FilesIndex(path=path), batch_class=face_batch.CelebrityBatch)

    def get_inference(self, dir_path, name, return_dummy=False, images_count=1):
        print('GET_INFERENCE CALLED')
        src = dir_path + name
        print('here is what i got ', src)

        dset = self.build_ds(src)
        print('dataset has been built', dset.indices)

        pred = self.find_neighbours_ppl << dset
        print('created pred')

        batch = pred.next_batch(1)
        print('got next batch')

        dst = name + '_cropped.png'
        imageio.imsave(dir_path + dst, batch.images[0])
        print('saved cropped image to ', dst)

        knn_files = [('00000' + str(current))[-6:] + '.jpg' for current in batch.neighbours[0][:images_count]]

        result = dict(dst=dst, knn=knn_files)
        if return_dummy:
            dummy_knn = self.index.dummy_k_neighbors(images_count, self.file_indices, \
                                                     self.saved_embeddings, batch.embedding[0])
            result['dummy_knn'] = [('00000' + str(current))[-6:] + '.jpg' for current in dummy_knn]
        return result
