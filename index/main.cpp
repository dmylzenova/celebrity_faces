#include "lsh.h"
#include <cassert>

int main() {
    // build index
    LSH lsh(2, 4, 13);
    std::vector<double> a = {-0.00301999, 0.0510758, 0.09474665, -0.02792699, -0.09574845, 0.00486881,
                             -0.09962147,  0.12455593,  0.05827475,  0.03300984,  0.02116934, 0.17401986, -0.1265259};
    std::vector<double> b = {-0.00301999, 0.0510758, 0.09474665, -0.02492699, -0.09574845, 0.00486881,
                             -0.09834247,  0.12455593,  0.05827475,  0.03300984,  0.02116934, 0.10401986, -0.1265259};
    std::vector<double> c = {-0.00301999, 0.0510758, 0.09474665, -0.02492697, -0.09574845, 0.01486881,
                             -0.10862047,  0.12455593,  0.05827475,  0.13300984,  0.02116934, 0.10401986, -0.1265259};
    std::vector<double> d = {-0.01301999, 0.0510758, 0.09474665, -0.02592699, -0.19574845, 0.00486881,
                             -0.09862047,  0.12455593,  0.05827475,  0.03300984,  0.02116934, 0.10401986, -0.1269259};
    std::vector<double> e = {-0.10301999, 0.0510758, 0.09474665, -0.02892699, -0.19574945, 0.00486881,
                             -0.09862047,  0.12455593,  0.05827475,  0.03340984,  0.02216934, 0.10401986, -0.1265259};

    std::vector<std::vector<double>> points(4);
    points[0] = a;
    points[1] = b;
    points[2] = c;
    points[3] = d;
    lsh.create_splits();
    assert (lsh.write_planes_to_file("./planes.txt"));

    lsh.add_to_table(1, a);
    lsh.add_to_table(5, b);
    lsh.add_to_table(2, c);
    lsh.add_to_table(3, d);
    assert (lsh.write_hash_tables_to_files("./"));
    assert (lsh.write_index_embedding_dict("./index_embedding.txt"));


    // find neighbors
    LSH lsh1(2, 4, 13);
    assert (lsh1.fill_data_from_files("./planes.txt", "./", "./index_embedding.txt"));
    std::vector<int> answer = lsh1.find_k_neighbors(2, e);

    // dummy k_neighbors

    std::vector<int> indexes;
    std::vector<std::vector<double> > embeddings;

    indexes.push_back(1);
    indexes.push_back(5);
    indexes.push_back(2);
    indexes.push_back(3);
    embeddings.push_back(a);
    embeddings.push_back(b);
    embeddings.push_back(c);
    embeddings.push_back(d);

    std::vector<int> answer1 = lsh.dummy_k_neighbors(2, indexes, embeddings, e);

    // test asser
//    LSH lsh2(2, 65, 13);

    return 0;
}
