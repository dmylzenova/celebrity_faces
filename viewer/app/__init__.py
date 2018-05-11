# -*- coding: utf-8 -*-
from flask import Flask
from flask_uploads import UploadSet, configure_uploads, IMAGES

app = None
photos = None


def init_app(uploaded_img_path):
    global app, photos

    app = Flask(__name__)
    photos = UploadSet('photos', IMAGES)

    app.config['SECRET_KEY'] = 'dfsdfvwn56vesvnw4fcyvhn5746v83w5'
    app.config['UPLOADED_PHOTOS_DEST'] = uploaded_img_path
    configure_uploads(app, photos)

    # Тут к app привязываются все view_functions
    from app import views
