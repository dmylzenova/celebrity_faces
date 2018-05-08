from distutils.core import setup, Extension
from Cython.Build import cythonize


ext = Extension(
    "pylsh",
    sources=["lsh.pyx", "lsh.cpp"],
    language="c++",
)

setup(
    name="pylsh",
    ext_modules=cythonize(ext, language='c++'),
)