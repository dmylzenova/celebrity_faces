# celebrity_faces

To compile cython extensions put:

```bash
python3 setup.py build_ext --inplace
```
After compilation you get pylsh.so file.

To use it, type something like this:
```python
import pylsh

print(pylsh.PyLSH)
# <class 'pylsh.PyLSH'>
```
