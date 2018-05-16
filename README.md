# celebrity_faces

![alt text](http://url/to/img.png)https://www.meme-arsenal.com/memes/7200565e5919d7525d4401da11655f05.jpg)

При реализации нашего проекта мы прошли через 3 этапа:

1. Индекс для приближенного поиска соседей на С++
2. Рабочий пайплайн, принимающий на вход картинку и возвращающий K ближайших соседей на питоне
3. Докер-контейнер, на котором развернут сайт.

Подробнее о каждом из пунктов:
to be updated
1. Мы реализовали старый добрый LSH c косинусным расстоянием. Казалось бы, он достаточно прозрачен и накосячить в нем сложно. Но нам это удалось! Мы тестировали и смотрели на него, пока из наших глаз не потекли слезы, но все-таки нашли ошибку (ошибки, ошибки) в коде, которая отравляла всю нашу жизнь.
	В остальном - LSH изящен и прост, о нем нам рассказывали на лекции, может оказаться полезен в любой ситуации (т.е. метрике).

2. Нам хотелось, чтобы рабочий пайплайн был похож на рабочий пайплайн, а не на простыню всем известного кода, поэтому мы воспользовались библиотекой dataset, 	благодаря которой наш пайплайн выглядит так:

	```python
	self.find_neighbours_ppl = (Pipeline()
                                         .load(fmt='image', components='images')
                                         .to_array()
                                         .to_cv(src='images')
                                         .detect_face()
                                         .crop_from_bbox()
                                         .resize(IMG_SHAPE, fmt='cv')
                                         .to_rgb()
                                         .init_variable('predicted_embeddings', init_on_each_run=0)
                                         .init_model('static', MyModel, model_name,
                                                     config={'load' : {'path' : model_path, 'graph': model_name + '.meta', 'checkpoint' : checkpoint_path},  'build': False})
                                         .predict_model(model_name, fetches="embeddings:0",
                                                        feed_dict={'input:0' : B('images'), 'phase_train:0' : False},
                                                        save_to=B('embedding'), mode='w')
                                         .find_nearest_neighbours(src=index_path, k_neighbours=K_NEIGHBOURS))
    ```

    Хотя в нашем случае пайплайн очень прост и, возможно, не нуждался в визуальном упрощении, благодаря данному подходу код было очень удобно рефакторить и тестировать.
    Для предсказания эмбедингов мы использовали предобученный Inception ResNet v1 (facenet).
    Мы обрезаем входную картинку хааркаскадом из opencv. Для того чтобы сравнивать лица в одном масштабе, мы точно так же обрезали картинки из нашей базы. 
    3. Самой приключенческой задачей было развернуть сайт. Мы прошли от заглядывания на красивости фронтэндов до простых, но понятных решений, разрушили ореол загадочности в восприятии людей, поднимающих на постоянной основе докеры и пополнили список клиентов амазона.
    Неизвестно, с чем это связано, но мы не раз сталкивались с отсутствием доступа к амазоновскому серверу без vpn. Чтобы иметь возможность поднять за несколько минут все решение на новом инстансе, мы написали скрипт, скачивающий данные и запускающий наше решение.

    Можно запустить такой командой:
    ```
    wget 'https://raw.githubusercontent.com/dmylzenova/celebrity_faces/master/init2.sh' -O init2.sh && chmod +x ./init2.sh && ./init2.sh
    ```

# Оценка качества:
	В текущей конфигурации используется lsh c 50 хэш-таблицами и 64 гиперплоскостями.
	Наш поиск работает в 500 раз быстрее полного перебора и угадывает 4 из 7 соседей, подсчитанных полным перебором. 
