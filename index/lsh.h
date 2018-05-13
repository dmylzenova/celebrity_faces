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


struct embedding_type {
    int _image_index;
    std::vector<double> _emb;

};

class LSH {

private:
    int _num_hash_tables;
    int _num_splits;
    int _dimension_size;
    std::mt19937 _generator;
public:
    LSH();
    std::vector<std::vector<std::vector<double> > >_planes;
    std::vector<std::unordered_map<std::string, std::vector<embedding_type> > > _hash_tables;
    LSH(int num_hash_tables, int num_splits, int dimension_size, std::string path_to_dir);
    void write_map_to_file(std::string path_to_dir);
    struct sortbysecond {
        bool operator()(const std::pair<int, double> &a, const std::pair<int, double> &b) const {
            return a.second > b.second;
        }
    };
    double dot(const std::vector<double> &x, const std::vector<double> &y);
    std::vector<double> normalize(std::vector<double>& v);
    std::vector<double> multiply(std::vector<std::vector<double> > &matrix, std::vector<double> &v);
    double calculate_distance(std::vector<double>& v_first, std::vector<double>& v_sec);
    std::vector<std::vector<double> > create_splits_for_one_table(std::vector<std::vector<double> > points);
    void create_splits(std::vector<std::vector<double> > points, std::string path_to_dir);
    std::string get_hash(std::vector<double> point, int hash_table_index);
    void add_to_table(int index, std::vector<double> embedding);
    std::vector<int> find_k_neighboors(int k, std::vector<double> embedding);
    std::vector<int> dummy_k_neighboors(int k, int index, std::vector<int> indexes,
                                        std::vector<std::vector<double> > embeddings,
                                        std::vector<double> given_point);
};
