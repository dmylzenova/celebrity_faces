#!/usr/bin/env python

"""
setup.py file for SWIG AnnoyIndex
"""

from distutils.core import setup, Extension


annoy_module = Extension('_AnnoyIndex',
                           sources=['AnnoyIndex_wrap.cxx', 'AnnoyIndex.cpp'],
                           )

setup (name = 'AnnoyIndex',
       version = '0.1',
       author      = "SWIG Docs",
       description = """Simple swig example from docs""",
       ext_modules = [annoy_module],
       py_modules = ["AnnoyIndex"],
       )
