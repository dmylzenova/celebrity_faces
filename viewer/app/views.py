# -*- coding: utf-8 -*-
import os
import flask
from app import app, photos, mt


ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg'}


def allowed_file(filename):
    return '.' in filename and str.lower(filename.rsplit('.', 1)[1]) in ALLOWED_EXTENSIONS


@app.route('/')
@app.route('/index', methods=['GET', 'POST'])
def index():
    if flask.request.method == 'POST':
        if 'photo' in flask.request.files:
            cur_photo = flask.request.files['photo']
            if not allowed_file(cur_photo.filename):
                flask.flash(u'Недопустимый формат файла. Допустимые расширения: ' + ', '.join(ALLOWED_EXTENSIONS),
                            category='error')
            else:
                filename = photos.save(cur_photo)
                flask.flash(u'Фото загружено.', category='info')
                app.logger.error("loaded")
                return flask.redirect(flask.url_for("result", filename=filename))
        else:
            flask.flash(u'Файл не выбран.', category='error')

    return flask.render_template('index.html', title='Home', debug=app.debug)


@app.route('/result/<filename>')
def result(filename):
    init_photo_abs_dir = os.path.dirname(__file__) + "/static/uploaded_img/"
    if not os.path.isfile(init_photo_abs_dir + filename):
        flask.flash(u'Файл с именем "%s" не загружен. Сначала загрузи фото.' % filename, category='error')
        return flask.redirect(flask.url_for("index"))

    cropped_photo, celebrity_photos = mt.get_inference(init_photo_abs_dir, filename, images_count=8)
    return flask.render_template('result.html', init_photo=filename, cropped_photo=cropped_photo,
                                 celebrity_photos=celebrity_photos)
