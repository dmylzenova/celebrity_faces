%module AnnoyIndex

%{
#include "AnnoyIndex.hpp"
%}
 
 struct Random {
 public:
 	uint32_t x;
 	uint32_t y;
 	uint32_t z;
 	uint32_t c;
 	Random(uint32_t seed = 123456789);
 	uint32_t kiss();
 	inline int flip();
 	inline size_t index(size_t n);
 };

inline long long dot(const long long *x, const long long *y, int f);
inline double get_norm(long long *v, int f);
inline void normalize(long long *v, int f);
struct Node {
	long long n_descendants;
    long long a; // need an extra constant term to determine the offset of the plane
    union {
        long long children[2];
        long long norm;
    };
    long long v[1];	
    static inline double distance(const Node* x, const Node* y, int f);
    static inline void init_node(Node* n, int f);
    inline void two_means(const std::vector<Node *> &nodes, int f, Random &random, bool cosine, Node *p, Node *q);
    static inline double pq_distance(double distance, double margin, int child_nr);
    static inline double pq_initial_value();
    static inline void create_split(const std::vector<Node*>& nodes, int f, size_t s, Random& random, Node* n);
    static inline double normalized_distance(double distance);
    static inline double margin(const Node* n, const long long* y, int f);
    static inline bool side(const Node* n, const long long * y, int f, Random& random);
};

class AnnoyIndexInterface {
	public:
    virtual ~AnnoyIndexInterface() {};
    virtual void add_item(long long item, const double* w) = 0;
    virtual void build(int q) = 0;
    virtual void unbuild() = 0;
    virtual bool save(const char* filename) = 0;
    virtual void unload() = 0;
    virtual bool load(const char* filename) = 0;
    virtual double get_distance(long long i, long long j) = 0;
    virtual void get_nns_by_item(long long item, std::size_t n, std::size_t search_k, std::vector<double>* result,
                                 std::vector<double>* distances) = 0;
    virtual void get_nns_by_vector(const double* w, std::size_t n, std::size_t search_k, std::vector<double >* result,
                                   std::vector<double >* distances) = 0;
    virtual long get_n_items() = 0;
    virtual void verbose(bool v) = 0;
    virtual void get_item(long item, double* v) = 0;
    virtual void set_seed(int q) = 0;
}
class AnnoyIndex : public AnnoyIndexInterface {
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
    bool _loaded;
    bool _verbose;
    int _fd;

protected:
	void _allocate_size(long long n);
	inline Node* _get(long long i);
	long long _make_tree(const std::vector<long long>& indices, bool is_root);
	void _get_all_nns(const long long* v, size_t n, size_t search_k, std::vector<double>* result, std::vector<double>* distances);
};








