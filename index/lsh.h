#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <random>
#include <bitset>


struct embedding_type {
    int _image_index;
    std::vector<double> _emb;

};

class LSH {

public:
    LSH();
    LSH(size_t num_hash_tables, size_t num_splits, size_t dimension_size, std::string path_to_dir = "");

    void write_map_to_file(std::string path_to_dir);

    struct sortbysecond {
        bool operator()(const std::pair<int, double> &a, const std::pair<int, double> &b) const {
            return a.second <= b.second;
        }
    };

    double dot(const std::vector<double> &x, const std::vector<double> &y);
    std::vector<double> normalize(std::vector<double>& v);
    std::vector<double> multiply(std::vector<std::vector<double> > &matrix, std::vector<double> &v);
    double calculate_distance(std::vector<double>& v_first, std::vector<double>& v_sec);
    std::vector<std::vector<double> > create_splits_for_one_table();
    void create_splits(const std::string &path_to_dir);
    unsigned long long get_hash(std::vector<double> point, size_t hash_table_index);
    void add_to_table(int index, std::vector<double> embedding);
    std::vector<int> find_k_neighboors(size_t k, std::vector<double> embedding);
    std::vector<int> dummy_k_neighboors(size_t k, std::vector<int> indexes, std::vector<std::vector<double> > embeddings,
                                        std::vector<double> given_point);

protected:
    size_t _num_hash_tables;
    size_t _num_splits;
    size_t _dimension_size;;
    std::vector<std::vector<std::vector<double> > >_planes;
    std::vector<std::unordered_map<unsigned long long, std::vector<embedding_type> > > _hash_tables;

    std::mt19937 _generator;
};
