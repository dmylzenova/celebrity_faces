from distutils.core import setup, Extension
from Cython.Build import cythonize


ext = Extension(
    "pylsh",
    sources=["lsh.pyx", "lsh.cpp"],
    language="c++",
    extra_compile_args=["-stdlib=libc++"],
)

setup(
    name="pylsh",
    ext_modules=cythonize(ext, language='c++'),

)