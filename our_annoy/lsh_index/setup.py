from distutils.core import setup, Extension
from Cython.Build import cythonize


ext = Extension(
    "pylsh",
    sources=["lsh.pyx"],
    language="c++",
    extra_compile_args=["-std=c++11", "-std=gnu++11"],
)

setup(
    name="pylsh",
    ext_modules=cythonize(ext, language='c++'),
)

