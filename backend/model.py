import sys

import tensorflow as tf
# sys.path.append("..")
from dataset.dataset.models.tf import TFModel


class MyModel(TFModel):
    def __init__(self, *args, **kwargs):
        """ Call __init__ of TFModel. """
        super().__init__(*args, **kwargs)

    def predict(self, fetches=None, feed_dict=None):      # pylint: disable=arguments-differ
        """ Get predictions on the data provided

        Parameters
        ----------
        fetches : tuple, list
            a sequence of `tf.Operation` and/or `tf.Tensor` to calculate
        feed_dict : dict
            input data, where key is a placeholder name and value is a numpy value

        Returns
        -------
        Calculated values of tensors in `fetches` in the same structure

        Notes
        -----
        The only difference between `predict` and `train` is that `train` also executes a `train_step` operation
        which involves calculating and applying gradients and thus chainging model weights.

        See also
        --------
        `Tensorflow Session run <https://www.tensorflow.org/api_docs/python/tf/Session#run>`_
        """
        with self.graph.as_default():
            # _feed_dict = self._fill_feed_dict(feed_dict, is_training=False)
            _feed_dict = {}
            for placeholder, value in feed_dict.items():
                placeholder = tf.get_default_graph().get_tensor_by_name(placeholder)
                _feed_dict.update({placeholder: value})
            
            if isinstance(fetches, str):
                _fetches = tf.get_default_graph().get_tensor_by_name(fetches)
            elif isinstance(fetches, (tuple, list)):
                _fetches = []
                for fetch in fetches:
                    _fetches.append(tf.get_default_graph().get_tensor_by_name(fetch))                    
            # _fetches = self._fill_fetches(fetches, default='predictions')
            output = self.session.run(_fetches, _feed_dict)
        return self._fill_output(output, _fetches)

