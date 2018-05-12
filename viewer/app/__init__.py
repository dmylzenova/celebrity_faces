# -*- coding: utf-8 -*-
from flask import Flask
from flask_uploads import UploadSet, configure_uploads, IMAGES

app = None
photos = None
mt = None


def init_app(uploaded_img_path, model_path, model_name, checkpoint,
             cropped_photos_dir, index_path, int_mapping_file, haarcascade_xml_path):
    global app, photos, mt

    app = Flask(__name__)
    photos = UploadSet('photos', IMAGES)

    app.config['SECRET_KEY'] = 'dfsdfvwn56vesvnw4fcyvhn5746v83w5'
    app.config['UPLOADED_PHOTOS_DEST'] = uploaded_img_path
    configure_uploads(app, photos)

    import mt_controller
    mt = mt_controller.MtController(
        model_path=model_path,
        model_name=model_name,
        checkpoint=checkpoint,
        cropped_photos_dir=cropped_photos_dir,
        index_path=index_path,
        int_mapping_file=int_mapping_file,
        haarcascade_xml_path=haarcascade_xml_path
    )

    # Тут к app привязываются все view_functions
    from app import views
