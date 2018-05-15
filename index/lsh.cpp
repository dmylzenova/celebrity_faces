#include "lsh.h"
#include <cassert>


void write_planes_to_file(std::vector<std::vector<std::vector<double> > > planes, const std::string &path_to_dir) {
    std::ofstream file;
    file.open(path_to_dir + "planes.txt");
    for (size_t line = 0; line < planes.size(); ++line) {
        for (size_t row = 0; row < planes[0].size(); ++row) {
            for (size_t col = 0; col < planes[0][0].size(); ++col) {
                file << planes[line][row][col] << ' ';
            }
            file << '\n';
        }
        file << '\n';
    }
    file.close();
}


std::vector<std::vector<std::vector<double> > >
read_planes_from_file(size_t num_hash_tables, size_t num_splits, const std::string &path_to_dir) {
    std::vector<std::vector<std::vector<double> > > result(num_hash_tables);
    for (size_t i = 0; i < num_hash_tables; ++i) {
        result[i].resize(num_splits);
    }
    std::string line;
    std::ifstream file(path_to_dir + "planes.txt");
    int split_ind = 0;
    size_t hash_table_num = 0;
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.empty()) {
                hash_table_num += 1;
                split_ind = 0;
                if (hash_table_num == num_hash_tables)
                    break;
            } else {
                std::istringstream s2(line);
                double tmp;
                while (s2 >> tmp) {
                    result[hash_table_num][split_ind].push_back(tmp);
                }
                split_ind += 1;
            }
        }
        file.close();
    }
    return result;
}


std::unordered_map<unsigned long long, std::vector<embedding_type> > read_map_from_one_file(const std::string &name) {
    std::unordered_map<unsigned long long, std::vector<embedding_type> > result;
    std::string line;
    unsigned long long key = 0;
    bool new_key = true;
    size_t sz = 0;

    std::ifstream file(name);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.empty()) {
                new_key = true;
                getline(file, line);
                getline(file, line);
            }
            if (new_key) {
                key = std::stoull(line, &sz, 0);
                getline(file, line);
            }
            if (line.empty()) {
                break;
            }
            std::istringstream s2(line);
            int index;
            s2 >> index;

            double tmp;
            std::vector<double> emb;
            while (s2 >> tmp) {
                emb.push_back(tmp);
            }
            embedding_type point;
            point._image_index = index;
            point._emb = emb;
            result[key].push_back(point);
            new_key = false;
        }
    }
    file.close();
    return result;
}


std::vector<std::unordered_map<unsigned long long, std::vector<embedding_type> > >
read_map_from_files(size_t num_hash_tables, const std::string &path_to_dir) {
    std::vector<std::unordered_map<unsigned long long, std::vector<embedding_type> > > answer;
    for (size_t i = 0; i < num_hash_tables; ++i) {
        std::string name = path_to_dir + std::to_string(i) + ".txt";
        answer.push_back(read_map_from_one_file(name));
    }
    return answer;
};


LSH::LSH() = default;


LSH::LSH(size_t num_hash_tables, size_t num_splits, size_t dimension_size, std::string path_to_dir) {
    assert(num_splits <= 64);

    _num_hash_tables = num_hash_tables;
    _num_splits = num_splits;
    _dimension_size = dimension_size;

    if (path_to_dir.empty()) {
        _planes.resize(num_hash_tables);
        for (size_t i = 0; i < num_hash_tables; ++i) {
            _planes[i].resize(num_splits);
            for (size_t row = 0; row < num_splits; ++row) {
                _planes[i][row].resize(dimension_size);
            }
        }
        _hash_tables.resize(num_hash_tables);

    } else {
        _planes = read_planes_from_file(num_hash_tables, num_splits, path_to_dir);
        _hash_tables = read_map_from_files(num_hash_tables, path_to_dir);
    }
}

void LSH::write_map_to_file(std::string path_to_dir) {
    size_t num_hash_tables = this->_hash_tables.size();
    for (size_t i = 0; i < num_hash_tables; ++i) {
        std::string name = path_to_dir + std::to_string(i) + ".txt";
        std::ofstream file;
        file.open(name);
        auto it = this->_hash_tables[i].begin();
        while (it != this->_hash_tables[i].end()) {
            file << it->first;
            file << "\n";
            for (auto &item : it->second) {
                file << item._image_index << " ";
                for (double &el : item._emb) {
                    file << el << " ";
                }
                file << "\n";
            }
            file << "\n\n";
            it++;
        }
        file.close();
    }
}


double LSH::dot(const std::vector<double> &x, const std::vector<double> &y) {
    double sum = 0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        sum += x[i] * y[i];
    }
    return sum;
}


std::vector<double> LSH::normalize(std::vector<double> &v) {
    double norm = sqrt(dot(v, v));
    if (norm == 0) {
        return v;
    }
    for (double &i : v) {
        i /= norm;
    }
    return v;
}

std::vector<double> LSH::multiply(std::vector<std::vector<double> > &matrix, std::vector<double> &v) {
    std::vector<double> result;
    result.reserve(_num_splits);
    for (std::size_t row = 0; row < matrix.size(); ++row) {
        double sum = 0;
        for (std::size_t el = 0; el < matrix[0].size(); ++el) {
            sum += matrix[row][el] * v[el];
        }
        result.push_back(sum);
    }
    return result;
}


double LSH::calculate_distance(std::vector<double> &v_first, std::vector<double> &v_sec) {
    double distance = 0;
    double similarity = dot(v_first, v_sec) / sqrt(dot(v_first, v_first) * dot(v_sec, v_sec));
    distance = 1 - similarity;
    return distance;
}


std::vector<std::vector<double> > LSH::create_splits_for_one_table() {
    std::normal_distribution<float> distribution(0.0, 1.0);
    std::vector<std::vector<double> > planes;
    for (size_t i = 0; i < _num_splits; ++i) {
        std::vector<double> current_vector;
        for (size_t j = 0; j < _dimension_size; ++j) {
            double current_push = distribution(_generator);
            current_vector.push_back(current_push);
        }
        planes.push_back(current_vector);
    }
    return planes;
}


void LSH::create_splits(const std::string &path_to_dir) {
    for (size_t num_table = 0; num_table < _num_hash_tables; ++num_table) {
        this->_planes[num_table] = this->create_splits_for_one_table();
    }
    write_planes_to_file(_planes, path_to_dir);
}


unsigned long long LSH::get_hash(std::vector<double> point, size_t hash_table_index) {
    std::bitset<64> hash_value;
    std::vector<double> values = multiply(_planes[hash_table_index], point);
    for (size_t indx = 0; indx < _num_splits; ++indx) {
        hash_value[indx] = values[indx] > 0;
    }
    return hash_value.to_ullong();
}


std::vector<int> LSH::dummy_k_neighbors(size_t k, std::vector<int> indexes,
                                        std::vector<std::vector<double> > embeddings,
                                        std::vector<double> given_point) {
    std::vector<int> answer;
    std::vector<embedding_type> points;
    for (std::size_t i = 0; i < indexes.size(); ++i) {
        points.push_back({indexes[i], embeddings[i]});

    }
    std::vector<std::pair<int, double> > candidates;
    for (auto &point : points) {
        double curr_distance = calculate_distance(point._emb, given_point);
        candidates.emplace_back(std::make_pair(point._image_index, curr_distance));
    }
    std::sort(candidates.begin(), candidates.end(), sortbysecond());
    answer.push_back(candidates[0].first);
    for (size_t i = 1; i < k; ++i) {
        if (answer[i - 1] != candidates[i].first) {
            answer.push_back(candidates[i].first);
        }
    }
    return answer;
}


void LSH::add_to_table(int index, std::vector<double> embedding) {
    embedding_type point;
    point._image_index = index;
    point._emb = std::move(embedding);
    for (size_t i = 0; i < _num_hash_tables; ++i) {
        unsigned long long hash_val = get_hash(point._emb, i);
        this->_hash_tables[i][hash_val].push_back(point);
    }
}


std::vector<int> LSH::find_k_neighbors(size_t k, std::vector<double> embedding) {
    std::set<int> answer;
    std::vector<std::pair<int, double> > candidates;
    for (size_t i = 0; i < _num_hash_tables; ++i) {
        unsigned long long hash_value = get_hash(embedding, i);
        std::vector<embedding_type> result_points = _hash_tables[i][hash_value];
        for (auto &result_point : result_points) {
            candidates.emplace_back(std::make_pair(result_point._image_index,
                                                   calculate_distance(result_point._emb, embedding)));
        }
    }
    std::sort(candidates.begin(), candidates.end(), sortbysecond());
    if (candidates.empty()) {
        return {};
    }
    for (size_t i = 1; i < candidates.size(); ++i) {
        answer.insert(candidates[i].first);
        if (answer.size() == k) {
            break;
        }
    }
    std::vector<int> result(answer.begin(), answer.end());
    return result;
}
