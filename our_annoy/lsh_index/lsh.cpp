#include "lsh.h"


LSH::LSH() = default;

LSH::LSH(int num_hash_tables, int num_splits, int dimension_size) {
    _num_hash_tables = num_hash_tables;
    _num_splits = num_splits;
    _dimension_size = dimension_size;
    _planes.resize(num_hash_tables);
    for (int i = 0; i < num_hash_tables; ++i) {
        _planes[i].resize(num_splits);
        for (int row = 0; row < num_splits; ++row) {
            _planes[i][row].resize(dimension_size);
        }
    }
    _hash_tables.resize(num_hash_tables);
}

bool LSH::sortbysecond(const std::pair<std::vector<double>, int> &a, const std::pair<std::vector<double>,int> &b) {
    return (a.second > b.second);
}


double LSH:: dot(const std::vector<double> &x, const std::vector<double> &y) {
    double sum = 0;
    for (int i = 0; i < x.size(); ++i) {
        sum += x[i] * y[i];
    }
    return sum;
}


std::vector<double> LSH::normalize(std::vector<double>& v) {
    double norm = sqrt(dot(v, v));
    for (int i = 0; i < v.size(); ++i) {
        v[i] /= norm;
    }
    return v;
}

std::vector<double> LSH::multiply(std::vector<std::vector<double>> &matrix, std::vector<double> &v) {
    std::vector<double> result;
    result.reserve(v.size());
    for (int row = 0; row < matrix.size(); ++row) {
        double sum = 0;
        for (int el = 0; el < matrix[0].size(); ++el) {
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
    std::vector<std::vector<double> > plane(_num_splits);
    for (int i = 0; i < _num_splits; ++i) {
        plane[i].resize(_dimension_size);
    }
    for (int cur_split = 0; cur_split < _num_splits; ++cur_split) {
        srand(time(NULL)); // random seed
        int first_rand_ind = rand() % _dimension_size;
        int sec_rand_ind = rand() % _dimension_size;
        for (int i = 0; i < _dimension_size; ++i) {
            plane[cur_split][i] = points[sec_rand_ind][i] - points[first_rand_ind][i];
        }
        plane[cur_split] = normalize(plane[cur_split]);
    }
    return plane;
}


void LSH::create_splits(std::vector<std::vector<double> > points) {
    for (int num_table = 0; num_table < _num_hash_tables; ++num_table) {
        this->_planes[num_table] = this->create_splits_for_one_table(points);
    }
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


void LSH::add_to_table(std::vector<double> point) {
    for (int i = 0; i < _num_hash_tables; ++i) {
        this->_hash_tables[i][get_hash(point, i)] = point;
    }
}


std::vector<std::vector<double>> LSH::find_k_neighboors(int k, std::vector<double> point) {
    std::vector<std::vector<double>> answer(k);
    for(int i=0; i < k; ++i) {
        answer[i].resize(_dimension_size);
    }
    std::vector<std::pair<std::vector<double>, double>> candidates;
    candidates.resize(_num_hash_tables);
    for (int i = 0; i <_num_hash_tables; ++i) {
        std::string hash_value = get_hash(point, i);
        std::vector<double> result_point = _hash_tables[i][hash_value];
        candidates[i] = std::make_pair(result_point, calculate_distance(result_point, point));
    }
    std::sort(candidates.begin(), candidates.end(), sortbysecond);
    for (int i = 0; i < k; ++i) {
        answer[i] = candidates[i].first;
    }
    return answer;
}

