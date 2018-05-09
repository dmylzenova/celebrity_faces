# distutils: language=c++


from libcpp.vector cimport vector as cpp_vector
from libcpp cimport bool


ctypedef cpp_vector[double] cpp_vector_double
ctypedef cpp_vector[cpp_vector_double] cpp_vec_vec_double 



cdef extern from "lsh.h":
    cdef cppclass LSH:
        LSH() except +
        LSH(int, int, int)
        void create_splits(cpp_vec_vec_double)
        void add_to_table(cpp_vector_double)
        cpp_vec_vec_double find_k_neighboors(int, cpp_vector_double)


cdef class PyLSH:
    cdef LSH thisptr 
    def __cinit__(self, num_hash_tables, num_splits, dimension_size):
        self.thisptr = LSH(num_hash_tables, num_splits, dimension_size)

    def create_splits(self, points):
        return self.thisptr.create_splits(points)

    def add_to_table(self, point):
        return self.thisptr.add_to_table(point)

    def find_k_neighboors(self, k, point):
        return self.thisptr.find_k_neighboors(k, point)
