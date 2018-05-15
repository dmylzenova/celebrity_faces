import os
import imageio
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
        print('LSH initialized')
        a = '0.0194561 0.042469 -0.00241216 0.0404634 0.026206 0.0482399 0.0463853 -0.0908853 -0.0709574 -0.144772 0.011858 -0.114904 -0.0402228 0.100099 -0.0492029 -0.0944625 0.0665425 -0.0993312 0.00879607 -0.0796038 0.0261992 -0.181128 0.0627964 0.0843063 0.0743811 0.135361 0.112388 0.0272855 -0.0518486 -0.167158 -0.00187749 0.17207 -0.0719978 0.0496791 -0.0274812 0.0342841 0.151937 -0.124779 0.0700361 0.0733923 -0.0117331 -0.00160149 0.00506811 -0.0423161 -0.0740804 0.173151 0.0639334 0.0221061 0.0135171 -0.0809277 0.0553787 0.0043606 0.0919013 0.0685305 0.0240654 0.300156 0.0460886 -0.159905 0.168061 0.0932426 -0.0691807 0.0555146 0.0708056 0.0191332 -0.057153 0.0442914 0.118729 -0.0329558 0.0731806 0.102099 0.016923 0.133432 0.0202205 0.0725665 -0.117047 0.0847481 0.00099859 -0.0113888 -0.140756 -0.121368 -0.035942 0.0147354 0.00134443 0.0327282 0.126786 -0.0213281 0.0659621 -0.172813 0.112995 0.0858302 0.0137467 -0.101607 -0.0304303 -0.0391461 -0.0430552 0.185863 0.120424 -0.0756769 0.0509059 0.0491351 -0.0448453 -0.0902059 0.102599 -0.0472706 0.0404969 -0.026239 -0.162932 0.187223 0.0434553 -0.0994055 -0.0217782 -0.0192051 0.0020202 -0.0202612 0.0391378 0.113883 -0.0301008 0.104942 -0.0194257 -0.108668 0.0811891 -0.130431 0.0783285 0.0330896 0.00261154 -0.0231748 0.108669 -0.142922'
        if len(self.index.find_k_neighbors(5, list(map(float, a.split(' '))))) == 0:
            print('EMPTY INDEX')
        else:
            print('Successfully loaded LSH')

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

    def get_inference(self, dir_path, name, images_count=1):
        print('GET_INFERENCE CALLED')
        src = dir_path + name
        imageio.imread(src)

        print('here is what i got ', src)
        dset = self.build_ds(src)
        print('dataset has been built', dset.indices)
        pred = self.find_neighbours_ppl << dset
        print('created pred')
        batch = pred.next_batch(1)
        print('got next batch')
        cropped_image = batch.images[0]
        dst = name + '_cropped.png'
        imageio.imsave(dir_path + dst, cropped_image)
        print('saved cropped image to ', dst)
        knn_files = [str(current) + '.jpg' for current in batch.neighbours[0][:images_count]]
        return dst, knn_files
