# celebrity_faces

`Celebrity faces` is a project which supports a site where you can load your photo and get back photo of
a celebrity who looks the most like you.

## How does it work:
Project/face_batch.py contains class CelebrityBatch which has methods for data preproccesing, Index creation and search for k approximate nearest neghbours. At the moment it utilizes spotify's [Annoy](https://github.com/spotify/annoy).

CelebrityBatch class is inherited from [dataset's](https://github.com/analysiscenter/dataset) ImagesBatch 
for flexible and easy looking workflows.

An instance of CelebrityBatch represents data batch and stores components: `images`, `coordinates`, `embedding`, etc.

Thus, image preproccesing pipeline looks like this:

```python
preprocess_ppl = (Pipeline()
     .load_cv(src=src_images, fmt='cv', components='images')
     .detect_face()
     .crop_from_bbox()          
     .resize(IMG_SHAPE, fmt='cv')
     .to_rgb()
     .to_pil()
```

To load a model from disk we create

```python
init_model_ppl = (Pipeline()
                    .init_model('static', MyModel, model_name,
                                config={'load' : {'path' : model_path, 'graph': model_name + '.meta',
                                                  'checkpoint' : checkpoint_path}, 'build': False}))
```

And to create Index we merge two pipelines above and add predict function from Project/model.py and dump embeddings to the disk:

```python
predict_ppl = ((preprocess_ppl + init_model_ppl)
                .predict_model(model_name, fetches="embeddings:0",
                               feed_dict={'input:0' : B('images'), 'phase_train:0' : False},
                               save_to=B('embedding'), mode='w')
               .dump(dst='cropped_embeddings.csv', fmt='csv', mode='a', components=['embedding'], header=False)

```

This pipeline is lazy so we need to ask it to run:

```python
(predict_ppl << dset).run(BATCH_SIZE, n_epochs=1)
```
where dset is an instance of class dataset which supports indexing over current data.

Once we have saved embeddings we are ready to build an AnnoyIndex and save it to the disk:
```python
preprocess_ppl = (Pipeline()
     	.load(src='embeddings.csv', fmt='csv', components='embedding', index_col='file_name', post=load_func)
       .build_index()
```

Then when we get new images we do the preproccessing and call a function to search for the neighbours using existing Index:

```python
my_workflow = (preprocess_ppl.find_nearest_neighbours(src='my_index.ann', k_neighbours=6)
				<< Dataset(index=FilesIndex(path=image_path)), batch_class=CelebrityBatch))
```

And the neighbours indices will be saves to the batch's component `neighbours`.

## About our knn method:
Now we use spotify's [Annoy](https://github.com/spotify/annoy). But we’re implementing our Annoy version. At the current stage there is class `AnnoyIndex.cpp` with two main methods: ```_make_tree``` - function for building tree and ```_get_all_nns``` - for searching all the trees until have k candidates, but now this methods don’t work.

 For wrapping our C++ code and use it in python like this: ```from AnnoyIndex import AnnoyIndex``` we use `SWIG`. We added SWIG interface file `AnnoyIndex.i` and now it's possible to call all class methods.
 
 Our plans: now our ```AnnoyIndex``` doesn't work, also we use only `Euclidean distance`, so we can also use `Сosine distance` and compare them.

 Also we will compare our Annoy with implemented Annoy, HNSW and FAISS - compare working time of algorithms and accuracy. For accuracy we want to generate brute force correct answers (ordered topK nearest neighbours) for a subset and then compare it with our predictions with some rank metrics, say NDCG.
