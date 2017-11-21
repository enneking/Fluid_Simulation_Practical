
#ifndef COMPACT_N_SEARCH__HPP
#define COMPACT_N_SEARCH__HPP

#include <Eigen/Core>

#include <vector>
#include <array>
#include <iterator>

#include <unordered_map>
#include <cstdint>


//#define USE_ARRAY
static const int MAX_PPC = 60;

typedef double Real;



struct HashKey
{
    HashKey() = default;
    HashKey(int i, int j, int k)
        : k({{ i, j, k}})
    {}

    int& operator[](unsigned int i) { return k[i]; }
    int operator[](unsigned int i) const { return k[i]; }
    bool operator==(const HashKey &other) const
    {
        return k == other.k;
    }

    bool operator!=(const HashKey &other) const
    {
        return !(*this == other);
    }

    std::array<int, 3> k;
};

struct ParticleID
{
    unsigned int object_id;
    unsigned int index;

    bool operator==(ParticleID const& other)
    {
        return index == other.index && object_id == other.object_id;
    }
};

struct HashEntry
{
    HashEntry() 
#ifdef USE_ARRAY
        : n_ind(0) 
#endif
    {}
    HashEntry(ParticleID const& id)
#ifdef USE_ARRAY
        : n_ind(0)
#endif
    {
        add(id);
    }

    void add(ParticleID const& id)
    {
#ifdef USE_ARRAY
        if (n_ind >= MAX_PPC)
        {
            std::cout << "ERROR: Too many indices in cell." << std::endl;
            return;
        }
        indices[n_ind++] = id;
#else
        indices.push_back(id);
#endif
    }

    void erase(ParticleID const& id)
    {
#ifdef USE_ARRAY
        for (unsigned int j(0); j < n_ind; ++j)
        {
            if (indices[j] == id)
            {
                std::swap(indices[j], indices[--n_ind]);
                return;
            }
        }
#else
        indices.erase(std::find(indices.begin(), indices.end(), id));
#endif
    }

    unsigned int n_indices() const
    {
#ifdef USE_ARRAY
        return n_ind;
#else
        return static_cast<unsigned int>(indices.size());
#endif
    }

#ifdef USE_ARRAY
    std::array<ParticleID, MAX_PPC> indices;
    unsigned int n_ind;
#else
    std::vector<ParticleID> indices;
#endif
};

struct SpatialHasher
{
    std::size_t operator()(HashKey const& k) const
    {
        return 
            73856093 * k.k[0] ^ 
            19349663 * k.k[1] ^ 
            83492791 * k.k[2];
    }
};

class CompactNSearch;
class SPHDiscretization
{

public:

    unsigned int n_neighbors(unsigned int i) const 
    {
        return static_cast<unsigned int>(m_neighbors[i].size());
    }
    ParticleID const& neighbor(unsigned int i, unsigned int k) const 
    {
        return m_neighbors[i][k];
    }
    Eigen::Vector3d const& point(unsigned int i) const { return m_x[i]; }
    std::size_t n_points() const { return m_n; }

    bool is_moving() const { return m_is_moving; }
    bool is_neighborsearch_enabled() const { return m_search_neighbors; }

    void moving(bool v) { m_is_moving = v; }
    void enable_neighborsearch(bool v) { m_search_neighbors = v; }

    template <typename T>
    void sort_field(T* lst) const;

private:

    friend CompactNSearch;
    SPHDiscretization(Eigen::Vector3d* x_, std::size_t n_, bool is_moving_, bool search_neighbors_)
        : m_x(x_), m_n(n_), m_is_moving(is_moving_), m_neighbors(n_)
        , m_keys(n_, {
        std::numeric_limits<int>::lowest(),
        std::numeric_limits<int>::lowest(),
        std::numeric_limits<int>::lowest() })
        , m_search_neighbors(search_neighbors_)
    {
        m_old_keys = m_keys;
    }


    Eigen::Vector3d* m_x;
    std::size_t m_n;
    bool m_is_moving;
    bool m_search_neighbors;

    std::vector<std::vector<ParticleID>> m_neighbors;
    std::vector<HashKey> m_keys, m_old_keys;
    std::vector<unsigned int> m_sort_table;
};

template <typename T>
void SPHDiscretization::sort_field(T* lst) const
{
    std::vector<T> tmp(lst, lst + m_sort_table.size());
    std::transform(m_sort_table.begin(), m_sort_table.end(), 
        stdext::unchecked_array_iterator<T*>(lst),
        [&](int i){ return tmp[i]; });
}

class CompactNSearch
{

public:

    CompactNSearch(Real r, bool erase_emtpy_cells = false);

	unsigned int add_discretization(Eigen::Vector3d* x_, std::size_t n_,
        bool is_moving_, bool search_neighbors_) 
    { 
        m_discretizations.push_back({x_, n_, is_moving_, search_neighbors_}); 
        return static_cast<unsigned int>(m_discretizations.size() - 1);
    }
    std::vector<SPHDiscretization> const& discretizations() const 
    { 
        return m_discretizations; 
    }
    void neighborhood_search();
    void z_sort();

    double radius() const { return std::sqrt(m_r2); }
    void set_radius(double r) 
    { 
        m_r2 = r * r; 
        m_inv_cell_size = 1.0 / r;
        m_initialized = false;
    }

private:

    void init();
    void insert_sort(std::vector<HashKey>& keys);
    void update_hash_table_parallel();
    void update_hash_table_serial();
    void erase_empty_entries();
    void query();

    inline 
    HashKey cell_index(Eigen::Vector3d const& x);

private:

    std::vector<SPHDiscretization> m_discretizations;

    Real m_inv_cell_size;
    Real m_r2;
    std::unordered_map<HashKey, unsigned int, SpatialHasher> m_map;
    std::vector<HashEntry> m_entries;

    bool m_erase_empty_cells;
    bool m_initialized, m_sorted;

    //boost::lockfree::queue<unsigned int> m_update_queue;
};


#endif //COMPACT_N_SEARCH__HPP
