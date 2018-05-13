#include "lsh.h"



void write_planes_to_file(std::vector<std::vector<std::vector<double> > > planes, std::string path_to_dir) {
    std::ofstream file;
    file.open(path_to_dir + "planes.txt");
    for (int line = 0; line < planes.size(); ++line) {
        for (int row = 0; row < planes[0].size(); ++row) {
            for (int col = 0; col < planes[0][0].size(); ++col) {
                file << planes[line][row][col] << ' ';
            }
            file << '\n';
        }
        file << '\n';
    }
    file.close();
}


std::vector<std::vector<std::vector<double> > > read_planes_from_file(int num_hash_tables, int num_splits, std::string path_to_dir) {
    std::vector<std::vector<std::vector<double> > > result(num_hash_tables);
    for (int i =0; i < num_hash_tables; ++i) {
        result[i].resize(num_splits);
    }
    std::string line;
    std::ifstream file (path_to_dir + "planes.txt");
    int split_ind = 0;
    int hash_table_num = 0;
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.empty()) {
                hash_table_num +=1;
                split_ind = 0;
                if (hash_table_num == num_hash_tables)
                    break;
            }
            else {
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


std::unordered_map<std::string, std::vector<embedding_type> > read_map_from_one_file(std::string name) {
    std::ifstream file (name);
    std::unordered_map<std::string, std::vector<embedding_type> > result;
    std::string line;
    std::string key;
    bool prev_key = true;
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.empty()) {
                prev_key = true;
                getline(file, line);
                getline(file, line);
            }
            if (prev_key) {
                key = line;
                getline(file, line);
            }
            if (line.empty()) {
                break;
            }
            int index;
            index = line[0] - '0';
            line = line.erase(0, 1);
            double tmp;
            std::istringstream s2(line);
            std::vector<double> emb;
            while (s2 >> tmp) {
                emb.push_back(tmp);
            }
            embedding_type point;
            point._image_index = index;
            point._emb = emb;
            result[key].push_back(point);
            prev_key = false;

        }
    }
    file.close();
    return result;
}


std::vector<std::unordered_map<std::string, std::vector<embedding_type> > > read_map_from_files(int num_hash_tables, std::string path_to_dir) {
    std::vector<std::unordered_map<std::string, std::vector<embedding_type> > > answer;
    for (int i = 0; i < num_hash_tables; ++i) {
        std::string name = path_to_dir + std::to_string(i) + ".txt";
        answer.push_back(read_map_from_one_file(name));
    }
    return answer;
};


LSH::LSH() = default;


LSH::LSH(int num_hash_tables, int num_splits, int dimension_size, std::string path_to_dir="") {
    _num_hash_tables = num_hash_tables;
    _num_splits = num_splits;
    _dimension_size = dimension_size;
    if (path_to_dir.empty()) {
        _planes.resize(num_hash_tables);
        for (int i = 0; i < num_hash_tables; ++i) {
            _planes[i].resize(num_splits);
            for (int row = 0; row < num_splits; ++row) {
                _planes[i][row].resize(dimension_size);
            }
        }
        _hash_tables.resize(num_hash_tables);

    }
    else {
        _planes = read_planes_from_file(num_hash_tables, num_splits, path_to_dir);
        _hash_tables = read_map_from_files(num_hash_tables, path_to_dir);
    }
}

void LSH::write_map_to_file(std::string path_to_dir) {
    int num_hash_tables = this->_hash_tables.size();
    for (int i = 0; i < num_hash_tables; ++i) {
        std::string name = path_to_dir + std::to_string(i) + ".txt";
        std::ofstream file;
        file.open(name);
        auto it = this->_hash_tables[i].begin();
        while(it != this->_hash_tables[i].end()) {
            file << it->first;
            file << "\n";
            for (auto item = it->second.begin(); item != it->second.end(); ++item) {
                file << item->_image_index << " ";
                for (auto el = item->_emb.begin(); el != item->_emb.end(); ++el) {
                    file << *el << " ";
                }
                file << "\n";
            }
            file << "\n\n";
            it++;
        }
        file.close();
    }
}


double LSH:: dot(const std::vector<double> &x, const std::vector<double> &y) {
    double sum = 0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        sum += x[i] * y[i];
    }
    return sum;
}


std::vector<double> LSH::normalize(std::vector<double>& v) {
    double norm = sqrt(dot(v, v));
    if (norm == 0) {
        return v;
    }
    for (std::size_t i = 0; i < v.size(); ++i) {
        v[i] /= norm;
    }
    return v;
}

std::vector<double> LSH::multiply(std::vector<std::vector<double> > &matrix, std::vector<double> &v) {
    std::vector<double> result;
    result.reserve(v.size());
    for (std::size_t row = 0; row < matrix.size(); ++row) {
        double sum = 0;
        for (std::size_t el = 0; el < matrix[0].size(); ++el) {
            sum += matrix[row][el] * v[el];
        }
        result.push_back(sum);
    }
    return result;
}


double LSH::calculate_distance(std::vector<double>& v_first, std::vector<double>& v_sec) {
    double pp = sqrt(dot(v_first, v_first));
    double qq = sqrt(dot(v_sec, v_sec));
    double pq = sqrt(dot(v_first, v_sec));
    return pp + qq - 2 * pq;
}


std::vector<std::vector<double> > LSH::create_splits_for_one_table(std::vector<std::vector<double> > points) {
    std::uniform_int_distribution<int64_t> distrib(0, points.size());
    std::vector<std::vector<double> > plane(_num_splits);
    for (int i = 0; i < _num_splits; ++i) {
        plane[i].resize(_dimension_size);
    }
    for (int cur_split = 0; cur_split < _num_splits; ++cur_split) {
        std::size_t first_rand_ind = distrib(_generator);
        std::size_t sec_rand_ind = distrib(_generator);
        for (int i = 0; i < _dimension_size; ++i) {
            plane[cur_split][i] = points[sec_rand_ind][i] - points[first_rand_ind][i];
        }
        plane[cur_split] = normalize(plane[cur_split]);
    }
    return plane;
}

std::vector<std::vector<double> > LSH::create_splits_other(std::vector<std::vector<double> > points) {
    std::normal_distribution<float> distribution(0.0, 1.0);
    std::vector<std::vector<double> > planes;
    for (int i = 0; i < _num_splits; ++i) {
        std::vector<double > current_vector;
        for (int j = 0; j < _dimension_size; ++j) {
            double current_push = distribution(_generator);
            current_vector.push_back(current_push);
        }
        planes.push_back(current_vector);
    }
    return planes;
}


void LSH::create_splits(std::vector<std::vector<double> > points, std::string path_to_dir) {
    for (int num_table = 0; num_table < _num_hash_tables; ++num_table) {
        //this->_planes[num_table] = this->create_splits_for_one_table(points);
        this->_planes[num_table] = this->create_splits_other(points);
    }
    write_planes_to_file(_planes, path_to_dir);
}


std::string LSH::get_hash(std::vector<double> point, int hash_table_index) {
    std::string hash_value;
    hash_value.resize(_dimension_size);
    std::vector<double> values = multiply(_planes[hash_table_index], point);
    for (int indx = 0; indx < _dimension_size; ++indx) {
        if (values[indx] > 0) {
            hash_value[indx] = '1';

        } else {
            hash_value[indx] = '0';
        }
    }
    return hash_value;
}


std::vector<int> LSH::dummy_k_neighboors(int k, int index, std::vector<int> indexes,
                                         std::vector<std::vector<double> > embeddings,
                                         std::vector<double> given_point) {
    std::vector<int> answer;
    std::vector<embedding_type> points;
    for (std::size_t i = 0; i < indexes.size(); ++i) {
        points.push_back({indexes[i], embeddings[i]});

    }
    std::vector<std::pair<int, double> > candidates;
    for (auto el = points.begin(); el != points.end(); ++el) {
        double curr_distance = calculate_distance(el->_emb, given_point);
        candidates.emplace_back(std::make_pair(el->_image_index, curr_distance));
    }
    std::sort(candidates.begin(), candidates.end(), sortbysecond());
    answer.push_back(candidates[0].first);
    for (int i = 1; i < k; ++i) {
        if (answer[i - 1] != candidates[i].first) {
            answer.push_back(candidates[i].first);
        }
    }
    return answer;
}


void LSH::add_to_table(int index, std::vector<double> embedding) {
    embedding_type point;
    point._image_index = index;
    point._emb = embedding;
    for (int i = 0; i < _num_hash_tables; ++i) {
        std::string hash_val = get_hash(point._emb, i);
        this->_hash_tables[i][hash_val].push_back(point);
    }
}


std::vector<int> LSH::find_k_neighboors(int k, std::vector<double> embedding) {
    std::vector<int> answer;
    std::vector<std::pair<int, double> > candidates;
    for (int i = 0; i < _num_hash_tables; ++i) {
        std::string hash_value = get_hash(embedding, i);
        std::vector<embedding_type> result_points = _hash_tables[i][hash_value];
        for (auto result_point = result_points.begin(); result_point != result_points.end(); ++result_point) {
            candidates.emplace_back(std::make_pair(result_point->_image_index,
                                                   calculate_distance(result_point->_emb, embedding)));
        }
    }
    std::sort(candidates.begin(), candidates.end(), sortbysecond());
    if (candidates.empty()) {
        return answer;
    }
    int number = 0;
    answer.push_back(candidates[0].first);
    for (int i = 1; i < candidates.size(); ++i) {
        if (answer[i - 1] != candidates[i].first) {
            answer.push_back(candidates[i].first);
            number++;
            if (number == k) {
                break;
            }
        }
    }
    return answer;
}
