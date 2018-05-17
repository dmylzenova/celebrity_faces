    #include "lsh.h"
    #include <cassert>
    #include <cstring>
    #include <functional>

    LSH::LSH() = default;


    LSH::LSH(size_t num_hash_tables, size_t num_splits, size_t dimension_size) {
        assert(num_splits <= 64);

        _num_hash_tables = num_hash_tables;
        _num_splits = num_splits;
        _dimension_size = dimension_size;

        _planes.resize(num_hash_tables);
        for (size_t i = 0; i < num_hash_tables; ++i) {
            _planes[i].resize(num_splits);
            for (size_t row = 0; row < num_splits; ++row) {
                _planes[i][row].resize(dimension_size);
            }
        }
        _hash_tables.resize(num_hash_tables);
    }

    void LSH::create_splits(std::vector<std::vector<double> > point) {
        for (size_t num_table = 0; num_table < _num_hash_tables; ++num_table) {
            this->_planes[num_table] = this->create_splits_for_one_table();
        }
    }

    bool LSH::write_planes_to_file(const std::string &path_to_file) {
        std::ofstream file;
        file.open(path_to_file);
        if (file.is_open()) {
            for (auto &hash_table_planes : _planes) {
                for (auto &plane : hash_table_planes) {
                    for (double val: plane) {
                        file << val << ' ';
                    }
                    file << std::endl;
                }
                file << std::endl;
            }
            file.close();
            return true;
        }
        std::cerr << "Error: " << std::strerror(errno) << std::endl;
        return false;
    }

    bool LSH::read_planes_from_file(const std::string &path_to_file) {
        std::string line;
        int split_index = 0;
        size_t hash_table_num = 0;
        std::ifstream file(path_to_file);
        if (file.is_open()) {
            while (getline(file, line)) {
                if (line.empty()) {
                    hash_table_num += 1;
                    split_index = 0;
                } else {
                    std::istringstream linestream(line);
                    double current_value;
                    size_t id = 0;
                    while (linestream >> current_value) {
                        _planes[hash_table_num][split_index][id] = current_value;
                        id += 1;
                    }
                    split_index += 1;
                }
            }
            file.close();
            return true;
        }
        std::cerr << "Error: " << std::strerror(errno) << std::endl;
        return false;
    }


    void LSH::add_to_table(size_t index, const std::vector<double> &embedding) {
        for (size_t hash_table_index = 0; hash_table_index < _num_hash_tables; ++hash_table_index) {
            unsigned long long hash_val = get_hash(embedding, hash_table_index);
            this->_hash_tables[hash_table_index][hash_val].insert(index);
        }
        _img_index_to_embedding.insert({index, embedding});
    }

    bool LSH::write_index_embedding_dict(const std::string &path_to_file) {
        std::ofstream file;
        file.open(path_to_file);
        if (file.is_open()) {
            for (auto &dict_record : _img_index_to_embedding) {
                file << dict_record.first << " ";
                for (auto &val: dict_record.second) {
                    file << val << " ";
                }
                file << std::endl;
            }
            file.close();
            return true;
        }
        std::cerr << "Error: " << std::strerror(errno) << std::endl;
        return false;
    }

    bool LSH::read_index_embedding_dict(const std::string &path_to_file) {
        size_t img_index;
        std::string line;
        std::ifstream file(path_to_file);
        if (file.is_open()) {
            while (getline(file, line)) {
                std::istringstream linestream(line);
                linestream >> img_index;

                double embedding_val;
                std::vector<double> embedding;
                while (linestream >> embedding_val) {
                    embedding.push_back(embedding_val);
                }
                _img_index_to_embedding.insert(std::make_pair(img_index, embedding));
            }
            file.close();
            return true;
        }
        std::cerr << "Error: " << std::strerror(errno) << std::endl;
        return false;
    }

    bool LSH::fill_data_from_files(const std::string &planes_path, const std::string &hash_tables_dir_path,
                                   const std::string &index_embedding_dict_path) {
        return this->read_planes_from_file(planes_path)
               && this->read_hash_tables_from_files(hash_tables_dir_path)
               && read_index_embedding_dict(index_embedding_dict_path);
    }

    bool LSH::write_hash_tables_to_files(const std::string &path_to_dir) {
        size_t num_hash_tables = this->_hash_tables.size();
        for (size_t i = 0; i < num_hash_tables; ++i) {
            std::string name = path_to_dir + std::to_string(i) + ".txt";
            std::ofstream file;
            file.open(name);
            if (file.is_open()) {
                for (auto &hash_img_ids: this->_hash_tables[i]) {
                    file << hash_img_ids.first << " ";
                    for (size_t img_index: hash_img_ids.second) {
                        file << img_index << " ";
                    }
                    file << std::endl;
                }
                file.close();
            } else {
                std::cerr << "Error: " << std::strerror(errno) << std::endl;
                return false;
            }
        }
        return true;
    }

    bool LSH::read_hash_table_from_file(std::unordered_map<unsigned long long, std::set<size_t> > *result,
                                        const std::string &file_name) {
        std::string line;
        unsigned long long lsh_hash = 0;

        std::ifstream file(file_name);
        if (file.is_open()) {
            while (getline(file, line)) {
                std::istringstream linestream(line);
                linestream >> lsh_hash;

                size_t cur_img_index;
                std::set<size_t> img_ids;
                while (linestream >> cur_img_index) {
                    img_ids.insert(cur_img_index);
                }

                result->insert(std::make_pair(lsh_hash, img_ids));
            }
            file.close();
            return true;
        }
        std::cerr << "Error: " << std::strerror(errno) << std::endl;
        return false;
    }

    bool LSH::read_hash_tables_from_files(const std::string &path_to_dir) {
        for (size_t i = 0; i < _num_hash_tables; ++i) {
            std::string name = path_to_dir + std::to_string(i) + ".txt";
            if (!LSH::read_hash_table_from_file(&_hash_tables[i], name)) {
                std::cerr << "Error: " << std::strerror(errno) << std::endl;
                return false;
            }
        };
        return true;
    };


    double LSH::dot(const std::vector<double> &x, const std::vector<double> &y) {
        assert (x.size() == y.size());
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
        for (std::size_t row = 0; row < matrix.size(); ++row) {
            double sum = 0;
            for (std::size_t el = 0; el < matrix[0].size(); ++el) {
                sum += matrix[row][el] * v[el];
            }
            result.push_back(sum);
        }
        return result;
    }


    double LSH::calculate_distance(const std::vector<double> &v_first, const std::vector<double> &v_sec) {
        double distance = 0;
        double similarity = dot(v_first, v_sec) / sqrt(dot(v_first, v_first) * dot(v_sec, v_sec));
        distance = 1 - similarity;
        return distance;
    }


    double LSH::calculate_euclidean_distance(const std::vector<double> &v_first, const std::vector<double> &v_sec) {
        assert(v_first.size() == v_sec.size());
        double sum = 0;
        for (size_t i=0; i < v_first.size(); ++i) {
            double delta = v_first[i] - v_sec[i];
            sum += delta * delta;
        }
        return sum > 0. ? sqrt(sum) : 0.;
    }


    // std::vector<std::vector<double> > LSH::create_splits_for_one_table(std::vector<std::vector<double> > points) {
    //     std::normal_distribution<float> distribution(0.0, 1.0);
    //     std::vector<std::vector<double> > planes;
    //     for (size_t i = 0; i < _num_splits; ++i) {
    //         std::vector<double> current_vector;
    //         for (size_t j = 0; j < _dimension_size; ++j) {
    //             double current_push = distribution(_generator);
    //             current_vector.push_back(current_push);
    //         }
    //         planes.push_back(current_vector);
    //     }
    //     return planes;
    // }


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



    unsigned long long LSH::get_hash(std::vector<double> point, size_t hash_table_index) {
        std::bitset<64> hash_value;
        std::vector<double> values = multiply(_planes[hash_table_index], point);
        for (size_t indx = 0; indx < _num_splits; ++indx) {
            hash_value[indx] = values[indx] > 0;
        }
        return hash_value.to_ullong();
    }


    std::vector<int> LSH::dummy_k_neighbors(size_t k, const std::vector<int> indexes,
                                            const std::vector<std::vector<double> > embeddings,
                                            const std::vector<double> given_point,
                                            bool use_euclidean) {
        std::function<double(const std::vector<double> &, const std::vector<double> &)> distance_func =
                use_euclidean ? &LSH::calculate_euclidean_distance : &LSH::calculate_distance;

        std::vector<int> answer;

        std::vector<std::pair<size_t, double> > candidates;
        for (size_t i = 0; i < indexes.size(); ++i) {
            double curr_distance = distance_func(embeddings[i], given_point);
            candidates.emplace_back(std::make_pair(indexes[i], curr_distance));
        }

        std::sort(candidates.begin(), candidates.end(), sortbysecond());
        for (auto &candidate : candidates) {
            answer.push_back(static_cast<int>(candidate.first));
            if (answer.size() == k) {
                break;
            }
        }
        return answer;
    }


    std::vector<int> LSH::find_k_neighbors(size_t k, const std::vector<double> embedding) {
        std::vector<int> answer;
        std::map<size_t , double> candidates;
        for (size_t hash_table_index = 0; hash_table_index < _num_hash_tables; ++hash_table_index) {
            unsigned long long hash_value = get_hash(embedding, hash_table_index);
            std::set<size_t > img_ids = _hash_tables[hash_table_index][hash_value];
            for (auto &cur_img_id : img_ids) {
                auto &candidate_img_embedding = _img_index_to_embedding.at(cur_img_id);
                if (candidates.find(cur_img_id) == candidates.end()) {
                    candidates.insert(std::make_pair(cur_img_id, calculate_distance(candidate_img_embedding, embedding)));
                }
            }
        }
        if (candidates.empty()) {
            return {};
        }
        std::vector<std::pair<size_t, double> > candidates_list(candidates.begin(), candidates.end());
        std::sort(candidates_list.begin(), candidates_list.end(), sortbysecond());

        for (auto &candidate : candidates_list) {
            answer.push_back(static_cast<int>(candidate.first));
            if (answer.size() == k) {
                break;
            }
        }
        return answer;
    }
