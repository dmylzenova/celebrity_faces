#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <random>
#include <bitset>


class LSH {

public:
    LSH();
    LSH(size_t num_hash_tables, size_t num_splits, size_t dimension_size);

    void create_splits();
    bool write_planes_to_file(const std::string &path_to_file);
    bool read_planes_from_file(const std::string &path_to_file);

    void add_to_table(size_t index, const std::vector<double> &embedding);
    bool write_index_embedding_dict(const std::string &path_to_file);
    bool read_index_embedding_dict(const std::string &path_to_dir);

    bool write_hash_tables_to_files(const std::string &path_to_dir);
    bool fill_data_from_files(const std::string &planes_path, const std::string &hash_tables_dir_path,
                              const std::string &index_embedding_dict_path);
    bool read_hash_tables_from_files(const std::string &path_to_dir);

    std::vector<std::vector<double> > create_splits_for_one_table();

    unsigned long long get_hash(std::vector<double> point, size_t hash_table_index);

    std::vector<int> find_k_neighbors(size_t k, std::vector<double> embedding);
    std::vector<int> dummy_k_neighbors(size_t k, std::vector<int> indexes,
                                       std::vector<std::vector<double> > embeddings,
                                       std::vector<double> given_point,
                                       bool use_euclidean=false);

    struct sortbysecond {
        bool operator()(const std::pair<size_t, double> &a, const std::pair<size_t, double> &b) const {
            return a.second <= b.second;
        }
    };

    static bool read_hash_table_from_file(std::unordered_map<unsigned long long, std::set<size_t> > *result,
                                          const std::string &file_name);

    static double calculate_distance(const std::vector<double>& v_first, const std::vector<double>& v_sec);
    static double calculate_euclidean_distance(const std::vector<double>& v_first, const std::vector<double>& v_sec);
    static std::vector<double> multiply(std::vector<std::vector<double> > &matrix, std::vector<double> &v);
    static double dot(const std::vector<double> &x, const std::vector<double> &y);
    static std::vector<double> normalize(std::vector<double>& v);

protected:
    size_t _num_hash_tables;
    size_t _num_splits;
    size_t _dimension_size;;
    // For each hash_table we have _num_splits vectors:
    std::vector<std::vector<std::vector<double> > >_planes;
    std::vector<std::unordered_map<unsigned long long, std::set<size_t>> > _hash_tables;
    std::map<size_t, std::vector<double>> _img_index_to_embedding;

    std::mt19937 _generator;
};
