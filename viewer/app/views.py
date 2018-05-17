# -*- coding: utf-8 -*-
import os
import flask
from . import forms
from app import app, photos, mt


@app.route('/')
@app.route('/index', methods=['GET', 'POST'])
def index():
    form = forms.InitForm()
    if flask.request.method == 'POST':
        if form.validate_on_submit():
            cur_photo = form.photo.data
            filename = photos.save(cur_photo)
            flask.flash(u'Фото загружено.', category='info')
            return flask.redirect(flask.url_for("result", filename=filename, show_dummy_knn=form.show_dummy_knn.data))
        else:
            flask.flash(u'Выберите фото с корректным расширением.', category='error')
    form.process()
    return flask.render_template('index.html', title='Home', debug=app.debug, form=form)


@app.route('/result/<filename>')
def result(filename):
    show_dummy_knn = flask.request.args.get('show_dummy_knn') == "True"

    init_photo_abs_dir = os.path.dirname(__file__) + "/static/uploaded_img/"
    if not os.path.isfile(init_photo_abs_dir + filename):
        flask.flash(u'Файл с именем "%s" не загружен. Сначала загрузи фото.' % filename, category='error')
        return flask.redirect(flask.url_for("index"))

    res = mt.get_inference(init_photo_abs_dir, filename, images_count=7, return_dummy=show_dummy_knn)
    return flask.render_template('result.html', init_photo=filename, cropped_photo=res["dst"],
                                 celebrity_photos=res["knn"], accurate_celebrity_photos=res.get("dummy_knn", []))
