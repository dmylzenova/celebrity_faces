# distutils: language=c++


from libcpp.vector cimport vector as cpp_vector
from libcpp cimport bool
from libcpp.string cimport string

ctypedef cpp_vector[double] cpp_vector_double
ctypedef cpp_vector[int] cpp_vector_int
ctypedef cpp_vector[cpp_vector_double] cpp_vec_vec_double


cdef extern from "lsh.h":
    cdef cppclass LSH:
        LSH() except +
        LSH(int, int, int, string)
        void write_map_to_file(string)
        void create_splits(string)
        void add_to_table(int, cpp_vector_double)
        cpp_vector_int find_k_neighboors(size_t, cpp_vector_double)
        cpp_vector_int dummy_k_neighboors(size_t, cpp_vector_int, cpp_vec_vec_double, cpp_vector_double)
        string get_hash(cpp_vector_double, size_t)
        double calculate_distance(cpp_vector_double&, cpp_vector_double&)


cdef class PyLSH:
    cdef LSH thisptr
    def __cinit__(self, num_hash_tables, num_splits, dimension_size, path_to_dir):
        self.thisptr = LSH(num_hash_tables, num_splits, dimension_size, path_to_dir)
    def write_map_to_file(self, path_to_dir):
        return self.thisptr.write_map_to_file(path_to_dir)
    def create_splits(self, path_to_dir):
        return self.thisptr.create_splits(path_to_dir)
    def add_to_table(self, index, embedding):
        return self.thisptr.add_to_table(index, embedding)
    def find_k_neighboors(self, k, point):
        return self.thisptr.find_k_neighboors(k, point)
    def dummy_k_neighboors(self, k, indexes, embeddings, given_point):
        return self.thisptr.dummy_k_neighboors(k, indexes, embeddings, given_point)
    def get_hash(self, point, hash_table_index):
        return self.thisptr.get_hash(point, hash_table_index)
    def calculate_distance(self, v_first, v_sec):
        return self.thisptr.calculate_distance(v_first, v_sec)
