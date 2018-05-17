"""Batch class for Celebrity faces project"""
import sys

import pylsh
import json

import cv2 as cv
import numpy as np
from annoy import AnnoyIndex
from dataset import ImagesBatch, action, inbatch_parallel, any_action_failed, DatasetIndex, F


class CelebrityBatch(ImagesBatch):
    components = 'images', 'coordinates', 'embedding', 'neighbours', 'int_indices', 'dummy_neighbours'

       
    @action
    @inbatch_parallel(init='indices', post='_assemble', target='for', components='dummy_neighbours')
    def load_dummy_neighbours(self, ix, src='dummy_neighbours.scv', k_neighbours=6):
        all_dummy = json.load(open(src, 'r'))
        return all_dummy[ix][:k_neighbours]
    
    @action
    def build_index(self, use_pylsh=True, pylsh_params=(5, 20, 128), dst_path='./index_data/', n_trees=10):
        """
        Builds and saves AnnoyIndex

        Parameters
        ----------
        dst_path : str
            a path to store built index
        pylsh_params : tuple of length 3
            pylsh_params
        n_trees : int
            number of trees used in AnnoyIndex
        """
        if use_pylsh:
            pyindex = pylsh.PyLSH(*params)
            pyindex.create_splits()
            for ix in self.indices:
                current_embd = self.get(ix, 'embedding')
                pyindex.add_to_table(int(ix), current_embd)
            pyindex.write_planes_to_file((dst_path + 'split.txt').encode(encoding='UTF-8'))
            pyindex.write_hash_tables_to_files((dst_path + 'index').encode(encoding='UTF-8'))
            pyindex.write_index_embedding_dict((dst_path + 'index_embedding.txt').encode(encoding='UTF-8'))
            print('saved Index to path', dst_path)

        else:
            n_dim = self.get(self.indices[0], 'embedding').shape[0]
            index = AnnoyIndex(n_dim)
            n_items = len(self.indices)
            for ix in range(n_items):
                index.add_item(ix, self.get(self.indices[ix], 'embedding'))
            index.build(n_trees)
            index.save(dst_path + 'annoy.ann')
            self.int_indices = list(range(n_items))
            print('saved Index to ', dst_path)
        return self

    @action
    @inbatch_parallel(init='indices', post='_assemble', target='for', components='neighbours')
    def find_nearest_neighbours(self, ix, pyindex='', use_pylsh=True, pylsh_params=(5, 2000, 128), src='my_index.ann', 
                                use_preloaded=True, k_neighbours=6):
        """
        Finds k approximate nearest neighbours using
        """
        embd = self.get(ix, 'embedding')
        if use_pylsh:
            print('HEEY', pyindex.find_k_neighbors(k_neighbours, embd))
            return pyindex.find_k_neighbors(k_neighbours, embd)
        else:
            saved_index = AnnoyIndex(embd.shape[0])
            saved_index.load(src)
            return saved_index.get_nns_by_vector(embd, k_neighbours)

    @action
    @inbatch_parallel(init='images', post='_assemble', components='coordinates')
    def detect_face(self, image, haarcascade_xml_path='haarcascade_frontalface_default.xml'):
        """
        Finds coordinates of the single face on the image using haarcascade from cv2.
        Parameters
        ----------
        image : np.array
            image with one face on it
        
        Returns
        -------
        bbox : tuple or list
            face's bounding box in format (x, y, w, h)
        """
        face_cascade = cv.CascadeClassifier(haarcascade_xml_path)
        gray = cv.cvtColor(image, cv.COLOR_BGR2GRAY)
        try:
            faces = face_cascade.detectMultiScale(gray, 1.3, 5)
        except Exception as e:
            print('FACE CASCADE ERROR ', e, 'image.shape is',  image.shape, 'gray shape is', gray.shape)
            return [0, 0, image.shape[1], image.shape[0]]
        if len(faces) > 1:
            return [0, 0, image.shape[1], image.shape[0]]
        try:
            return faces[0]
        except Exception as e:
            return [0, 0, image.shape[1], image.shape[0]]

    @action
    @inbatch_parallel(init='indices', post='_assemble', target='for', components='images')
    def crop_from_bbox(self, ix, margin=0.1, top_margin=2, src='images', dst='images', to_rgb=False, component_coord='coordinates'):
        """Crop area from an image using ``coordinates`` attribute
        Parameters
        ----------
        src : str
            data component's name
        dst : str
            the name of the component where the result will be recorded
        component_coord : str
            the name of the component with coordinates of the display with digits
        margin : float in  [0, 1]
            fraction of the face size to enlarge opencv bbox via each direction
        top_margin : float
            multiplier to enlarge margin on top of the face (to include hair, etc.)
        Returns
        -------
        self
        """
        image = self.get(ix, src)
        try:
            x, y, width, height = self.get(ix, component_coord)
        except Exception as e:
            print(e, self.get(ix, component_coord))
        w_margin = int(width * margin)
        h_margin = int(height * margin)
        
        left_y = max(0, y - h_margin * top_margin)
        right_y = min(image.shape[0], y + height + h_margin)
        
        left_x = max(0, x - w_margin)
        right_x = min(image.shape[1], x + width + w_margin)
        
        dst_data = image[left_y:right_y, left_x:right_x]
        if to_rgb:
            dst_data = dst_data[:, :, ::-1]
        return dst_data    
    
    @action
    @inbatch_parallel(init='images', target='for', post='_assemble', components='images')
    def to_rgb(self, image):
        return image[:, :, ::-1]
    
    @action
    @inbatch_parallel(init='images', post='_assemble', components='images')
    def to_cv(self, image, src='images'):
        return np.array(image)[:, :, ::-1]

    @action
    @inbatch_parallel(init='images', target='for', post='_assemble', components='images')
    def resize(self, image, *args, fmt='cv'):
        if fmt == 'cv':
            return cv.resize(image, *args)
        else:
            super().resize(image, *args)
            
    @action
    @inbatch_parallel(init='images', target='for', post='_assemble', components='images')
    def to_array(self, image):
        return np.array(image)[:, :, :3]

def load_func(data, fmt, components=None, *args, **kwargs):
    """Writes the data for components to a dictionary of the form:
    key : component's name
    value : data for this component
    Parameters
    ----------
    data : DataFrame
        inputs data
    fmt : strig
        data format
    components : list or str
        the names of the components into which the data will be loaded.
    Returns
    -------
    dict with keys - names of the compoents and values - data for these components.
    """
    # print('HERE')

    _ = fmt, args, kwargs
    _comp_dict = dict()
    for comp in components:
        if comp == 'embedding':
            data['tmp'] = data[data.columns[-1]].apply(lambda x: str(x).replace('[', '') \
                                                                                              .replace(']', '') \
                                                                                              .replace('', '')
                                                                                              .split(' '))
            _comp_dict[comp] = data['tmp'].apply(lambda x: np.array(list(map(float, [item for item in x if item != '']))))
        else:
            _comp_dict[comp] = data[comp].values.astype(str)
    return _comp_dict

