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
        LSH(int, int, int)
        bool fill_data_from_files(string, string, string)
        bool write_hash_tables_to_files(string)
        bool write_planes_to_file(string)
        bool write_index_embedding_dict(string)
        void create_splits()
        void add_to_table(int, cpp_vector_double)
        cpp_vector_int find_k_neighbors(size_t, cpp_vector_double)
        cpp_vector_int dummy_k_neighbors(size_t, cpp_vector_int, cpp_vec_vec_double, cpp_vector_double)
        unsigned long long get_hash(cpp_vector_double, size_t)
        double calculate_distance(cpp_vector_double&, cpp_vector_double&)


cdef class PyLSH:
    cdef LSH thisptr
    def __cinit__(self, num_hash_tables, num_splits, dimension_size):
        self.thisptr = LSH(num_hash_tables, num_splits, dimension_size)
    def fill_data_from_files(self, planes_path, hash_tables_dir_path, index_embedding_dict_path):
        return self.thisptr.fill_data_from_files(planes_path, hash_tables_dir_path, index_embedding_dict_path)
    def write_hash_tables_to_files(self, path_to_dir):
        return self.thisptr.write_hash_tables_to_files(path_to_dir)
    def write_index_embedding_dict(self, path_to_file):
        return self.thisptr.write_index_embedding_dict(path_to_file)
    def write_planes_to_file(self, path_to_file):
            return self.thisptr.write_planes_to_file(path_to_file)
    def create_splits(self):
        return self.thisptr.create_splits()
    def add_to_table(self, index, embedding):
        return self.thisptr.add_to_table(index, embedding)
    def find_k_neighbors(self, k, point):
        return self.thisptr.find_k_neighbors(k, point)
    def dummy_k_neighbors(self, k, indexes, embeddings, given_point):
        return self.thisptr.dummy_k_neighbors(k, indexes, embeddings, given_point)
    def get_hash(self, point, hash_table_index):
        return self.thisptr.get_hash(point, hash_table_index)
    def calculate_distance(self, v_first, v_sec):
        return self.thisptr.calculate_distance(v_first, v_sec)
