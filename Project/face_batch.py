"""Batch class for Celebrity faces project"""
import sys

import cv2 as cv
import numpy as np
from annoy import AnnoyIndex
sys.path.append('..')
from dataset import ImagesBatch, action, inbatch_parallel, any_action_failed, DatasetIndex, F

class CelebrityBatch(ImagesBatch):
    components = 'images', 'coordinates', 'embedding', 'neighbours', 'int_indices'

    @action
    def build_index(self, dst='my_index', n_trees=10):
        """
        Builds and saves AnnoyIndex

        Parameters
        ----------
        dst : str
            a path to store built index
        n_trees : int
            number of trees used in AnnoyIndex
        """
        n_dim = self.get(self.indices[0], 'embedding').shape[0]
        index = AnnoyIndex(n_dim)
        for ix in self.indices:
            index.add_item(int(ix), self.get(ix, 'embedding'))
        index.build(n_trees)
        index.save(dst + '.ann')
        print('saved Index to path', dst)
        
        return self

    @action
    @inbatch_parallel(init='indices', post='_assemble', target='threads', components='neighbours')
    def find_nearest_neighbours(self, ix, src='my_index.ann', k_neighbours=6):
        """
        Finds k approximate nearest neighbours using
        """
        embd = self.get(ix, 'embedding')
        saved_index = AnnoyIndex(embd.shape[0])
        saved_index.load(src)
        return saved_index.get_nns_by_vector(embd, k_neighbours)

    @action
    @inbatch_parallel(init='indices', post='_assemble', components='images')
    def load_cv(self, ix, src, fmt='cv', **kwargs):
        if fmt == 'cv':
            return cv.imread(src + str(ix) + '.jpg')
        else:
            raise ValueError('fmt must be cv')

    @action
    @inbatch_parallel(init='images', post='_assemble', components='coordinates')
    def detect_face(self, image):
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
        # face_cascade = cv.CascadeClassifier('..//opencv//data//haarcascades//haarcascade_frontalface_default.xml')
        face_cascade = cv.CascadeClassifier('haarcascade_frontalface_default.xml')
        gray = cv.cvtColor(image, cv.COLOR_BGR2GRAY)
        faces = face_cascade.detectMultiScale(gray, 1.3, 5)
        if len(faces) > 1:
            print('detect_face has found more than one face')
            raise ValueError
        try:
            return faces[0]
        except Exception as e:
            print('Unfortunately I couldn\'t detect face on your photo' , faces)

    @action
    @inbatch_parallel(init='indices', post='_assemble', target='for', components='images')
    def crop_from_bbox(self, ix, src='images', dst='images', to_rgb=False, component_coord='coordinates'):
        """Crop area from an image using ``coordinates`` attribute
        Parameters
        ----------
        src : str
            data component's name
        dst : str
            the name of the component where the result will be recorded
        component_coord : str
            the name of the component with coordinates of the display with digits
        Returns
        -------
        self
        """
        image = self.get(ix, src)
        try:
            x, y, width, height = self.get(ix, component_coord)
        except Exception as e:
            print(e, self.get(ix, component_coord))
        dst_data = image[y:y+height, x:x+width]
        if to_rgb:
            dst_data = dst_data[:, :, ::-1]
        return dst_data

    @action
    @inbatch_parallel(init='images', target='for', post='_assemble', components='images')
    def to_rgb(self, image):
        return image[:, :, ::-1]
    
    @action
    @inbatch_parallel(init='images', target='for', post='_assemble', components='images')
    def resize(self, image, *args, fmt='cv'):
        if fmt == 'cv':
            return cv.resize(image, *args)
        else:
            super().resize(image, *args)
            
    # @action
    # @inbatch_parallel(init='indices', target='for', post='_assemble2')
    # def reassemble_component(self, ix, component='embedding'):
    #     return (self.get(ix, 'embedding'), )


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
