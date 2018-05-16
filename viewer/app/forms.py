# -*- coding: utf-8 -*-
from flask_wtf import FlaskForm
from flask_wtf.file import FileField, FileRequired, FileAllowed
from wtforms import BooleanField
from app import photos


class InitForm(FlaskForm):
    photo = FileField("photo", validators=[FileRequired(), FileAllowed(photos, 'Images only!')])
    show_dummy_knn = BooleanField("show_dummy_knn", default=True)
