/*
Summary: Annoy’s algorithm

Preprocessing time:

Build up a bunch of binary trees. For each tree, split all points recursively by random hyperplanes.
Query time:

Insert the root of each tree into the priority queue
Until we have _search_k _candidates, search all the trees using the priority queue
Remove duplicate candidates
Compute distances to candidates
Sort candidates by distance
Return the top ones

*/

#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include <queue>
#include <limits>
#include <vector>
#include <iostream>
using std::numeric_limits;


// RANDOM

struct Random {
public:
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t c;

    Random(uint32_t seed = 123456789) {
        x = seed;
        y = 362436000;
        z = 521288629;
        c = 7654321;
    }

    uint32_t kiss() {
        // Linear congruence generator
        x = 69069 * x + 12345;

        // Xor shift
        y ^= y << 13;
        y ^= y >> 17;
        y ^= y << 5;

        // Multiply-with-carry
        uint64_t t = 698769069ULL * z + c;
        c = t >> 32;
        z = (uint32_t) t;

        return x + y + z;
    }

    inline int flip() {
        return kiss() & 1;
    }

    inline size_t index(size_t n) {
        // Draw random integer between 0 and n-1 where n is at most the number of data points you have
        return kiss() % n;
    }
};


inline long long dot(const long long *x, const long long *y, int f) {
    long long s = 0;
    for (int z = 0; z < f; z++) {
        s += (*x) * (*y);
        x++;
        y++;
    }
    return s;
}

inline double get_norm(long long *v, int f) {
    return sqrt(dot(v, v, f));
}

inline void normalize(long long *v, int f) {
    double norm = get_norm(v, f);
    if (norm > 0) {
        for (int z = 0; z < f; z++)
            v[z] /= norm;
    }
}

// DISTANCE


struct Node {
    long long n_descendants;
    long long a; // need an extra constant term to determine the offset of the plane
    long long children[2];
    long long norm;
    long long v[1];
};

static inline double distance(const Node* x, const Node* y, int f) {
    long long pp = x->norm ? x->norm : dot(x->v, x->v,
                                           f); // For backwards compatibility reasons, we need to fall back and compute the norm here
    long long qq = y->norm ? y->norm : dot(y->v, y->v, f);
    long long pq = dot(x->v, y->v, f);
    return pp + qq - 2 * pq;
}

static inline void init_node(Node* n, int f) {
    n->norm = dot(n->v, n->v, f);
}


inline void two_means(const std::vector<Node *> &nodes, int f, Random &random, bool cosine, Node *p, Node *q) {
    /*
      This algorithm is a huge heuristic. Empirically it works really well, but I
      can't motivate it well. The basic idea is to keep two centroids and assign
      points to either one of them. We weight each centroid by the number of points
      assigned to it, so to balance it.
    */
    static int iteration_steps = 200;
    size_t count = nodes.size();

    size_t i = random.index(count);
    size_t j = random.index(count - 1);
    j += (j >= i); // ensure that i != j
    memcpy(p->v, nodes[i]->v, f * sizeof(double));
    memcpy(q->v, nodes[j]->v, f * sizeof(double));
    if (cosine) {
        normalize(p->v, f);
        normalize(q->v, f);
    }
    init_node(p, f);
    init_node(q, f);

    int ic = 1, jc = 1;
    for (int l = 0; l < iteration_steps; l++) {
        size_t k = random.index(count);
        double di = ic * distance(p, nodes[k], f),
                dj = jc * distance(q, nodes[k], f);
        double norm = cosine ? get_norm(nodes[k]->v, f) : 1.0;
        if (norm <= double(0)) {
            continue;
        }
        if (di < dj) {
            for (int z = 0; z < f; z++)
                p->v[z] = (p->v[z] * ic + nodes[k]->v[z] / norm) / (ic + 1);
            init_node(p, f);
            ic++;
        } else if (dj < di) {
            for (int z = 0; z < f; z++)
                q->v[z] = (q->v[z] * jc + nodes[k]->v[z] / norm) / (jc + 1);
            init_node(q, f);
            jc++;
        }
    }
}

static inline double pq_distance(double distance, double margin, int child_nr) {
    if (child_nr == 0)
        margin = -margin;
    return std::min(distance, margin);
}

static inline double pq_initial_value() {
    return numeric_limits<double>::infinity();
}


static inline void create_split(const std::vector<Node*>& nodes, int f, size_t s, Random& random, Node* n) {
    Node* p = (Node*)malloc(s); // TODO: avoid
    Node* q = (Node*)malloc(s); // TODO: avoid
    two_means(nodes, f, random, false, p, q);

    for (int z = 0; z < f; z++)
        n->v[z] = p->v[z] - q->v[z];
    normalize(n->v, f);
    n->a = 0.0;
    for (int z = 0; z < f; z++)
        n->a += -n->v[z] * (p->v[z] + q->v[z]) / 2;
    free(p);
    free(q);
}

static inline double normalized_distance(double distance) {
    return sqrt(std::max(distance, double(0)));
}


static inline double margin(const Node* n, const long long* y, int f) {
    return n->a + dot(n->v, y, f);
}


static inline bool side(const Node* n, const long long * y, int f, Random& random) {
    long long dot = margin(n, y, f);
    if (dot != 0)
        return (dot > 0);
    else
        return random.flip();
}





class AnnoyIndex {
public:
    Node node;

protected:
    const int _f;
    size_t _s;
    long long _n_items;
    Random _random;
    void* _nodes; // Could either be mmapped, or point to a memory buffer that we reallocate
    long long _n_nodes;
    long long _nodes_size;
    std::vector<long long> _roots;
    long long _K;
    bool _verbose;
    int _fd;

public:
    
    AnnoyIndex(int f) : _f(f), _random() {
        _s = offsetof(Node, v) + f * sizeof(double); // Size of each node
        _verbose = false;
        _K = (_s - offsetof(Node, children)) / sizeof(long long); // Max number of descendants to fit into node
        reinitialize(); // Reset everything
    }

    ~AnnoyIndex() {
        unload();
    }


    void reinitialize() {
        _fd = 0;
        _nodes = NULL;
        _n_items = 0;
        _n_nodes = 0;
        _nodes_size = 0;
        _roots.clear();
    }

    void unload() {
        if (_fd) {
            off_t size = _n_nodes * _s;
        } else if (_nodes) {
            free(_nodes);
        }
        reinitialize();
        if (_verbose) std::cout << "Unloaded";
    }

protected:

    void _allocate_size(long long n) {
        if (n > _nodes_size) {
            const double reallocation_factor = 1.3;
            long long new_nodes_size = std::max(n,
                                        (long long)((_nodes_size + 1) * reallocation_factor));
            if (_verbose) std::cout << "Reallocating to " << new_nodes_size;
            _nodes = realloc(_nodes, _s * new_nodes_size);
            memset((char *)_nodes + (_nodes_size * _s) / sizeof(char), 0, (new_nodes_size - _nodes_size) * _s);
            _nodes_size = new_nodes_size;
        }
    }

    inline Node* _get(long long i) {
        return (Node*)((uint8_t *)_nodes + (_s * i));
    }


    long long _make_tree(const std::vector<long long>& indices, bool is_root) {
        if (indices.size() == 1 && !is_root)
            return indices[0];

        if (indices.size() <= (size_t)_K) {
            _allocate_size(_n_nodes + 1);
            long long item = _n_nodes++;
            Node* m = _get(item);
            m->n_descendants = is_root ? _n_items : (long long)indices.size();

            memcpy(m->children, &indices[0], indices.size() * sizeof(long long));
            return item;
        }

        std::vector<Node*> children;
        for (size_t i = 0; i < indices.size(); i++) {
            long long j = indices[i];
            Node* n = _get(j);
            if (n)
                children.push_back(n);
        }

        std::vector<long long> children_indices[2];
        Node* m = (Node*)malloc(_s); // TODO: avoid
        create_split(children, _f, _s, _random, m);

        for (size_t i = 0; i < indices.size(); i++) {
            long long j = indices[i];
            Node* n = _get(j);
            if (n) {
                bool side_val = side(m, n->v, _f, _random);
                children_indices[side_val].push_back(j);
            }
        }

        // If we didn't find a hyperplane, just randomize sides as a last option
        while (children_indices[0].size() == 0 || children_indices[1].size() == 0) {
            if (_verbose && indices.size() > 100000)
                std::cout << "Failed splitting items";

            children_indices[0].clear();
            children_indices[1].clear();

            // Set the vector to 0.0
            for (int z = 0; z < _f; z++)
                m->v[z] = 0.0;

            for (size_t i = 0; i < indices.size(); i++) {
                long long j = indices[i];
                children_indices[_random.flip()].push_back(j);
            }
        }

        int flip = (children_indices[0].size() > children_indices[1].size());

        m->n_descendants = is_root ? _n_items : (long long)indices.size();
        for (int side = 0; side < 2; side++)
            // run _make_tree for the smallest child first (for cache locality)
            m->children[side^flip] = _make_tree(children_indices[side^flip], false);

        _allocate_size(_n_nodes + 1);
        long long item = _n_nodes++;
        memcpy(_get(item), m, _s);
        free(m);

        return item;
    }

    void _get_all_nns(const long long* v, size_t n, size_t search_k, std::vector<double>* result,
                      std::vector<double>* distances) {
        Node* v_node = (Node *)malloc(_s); // TODO: avoid
        memcpy(v_node->v, v, sizeof(double)*_f);
        init_node(v_node, _f);

        std::priority_queue<std::pair<double, long long> > q;

        if (search_k == (size_t)-1)
            search_k = n * _roots.size(); // slightly arbitrary default value

        for (size_t i = 0; i < _roots.size(); i++) {
            q.push(std::make_pair(pq_initial_value(), _roots[i]));
        }

        std::vector<long long> nns;
        while (nns.size() < search_k && !q.empty()) {
            const std::pair<double , long long>& top = q.top();
            double d = top.first;
            long long i = top.second;
            Node* nd = _get(i);
            q.pop();
            if (nd->n_descendants == 1 && i < _n_items) {
                nns.push_back(i);
            } else if (nd->n_descendants <= _K) {
                const long long* dst = nd->children;
                nns.insert(nns.end(), dst, &dst[nd->n_descendants]);
            } else {
                double margin_val = margin(nd, v, _f);
                q.push(std::make_pair(pq_distance(d, margin_val, 1), nd->children[1]));
                q.push(std::make_pair(pq_distance(d, margin_val, 0), nd->children[0]));
            }
        }

        // Get distances for all items
        // To avoid calculating distance multiple times for any items, sort by id
        std::sort(nns.begin(), nns.end());
        std::vector<std::pair<double, long long> > nns_dist;
        long long last = -1;
        for (size_t i = 0; i < nns.size(); i++) {
            long long j = nns[i];
            if (j == last)
                continue;
            last = j;
            if (_get(j)->n_descendants == 1)  // This is only to guard a really obscure case, #284
                nns_dist.push_back(std::make_pair(distance(v_node, _get(j), _f), j));
        }

        size_t m = nns_dist.size();
        size_t p = n < m ? n : m; // Return this many items
        std::partial_sort(nns_dist.begin(), nns_dist.begin() + p, nns_dist.end());
        for (size_t i = 0; i < p; i++) {
            if (distances)
                distances->push_back(normalized_distance(nns_dist[i].first));
            result->push_back(nns_dist[i].second);
        }
        free(v_node);
    }

};
