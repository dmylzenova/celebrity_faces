# -*- coding: utf-8 -*-
from flask import Flask
from flask_uploads import UploadSet, configure_uploads, IMAGES

app = Flask(__name__)

photos = UploadSet('photos', IMAGES)

app.config['SECRET_KEY'] = 'dfsdfvwn56vesvnw4fcyvhn5746v83w5'
app.config['UPLOADED_PHOTOS_DEST'] = 'app/static/uploaded_img'
configure_uploads(app, photos)
