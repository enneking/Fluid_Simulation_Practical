
#include "CompactNSearch.h"
#include "z_sort_table.h"

#include <iostream>
#include <ppl.h>
#include <numeric>


using namespace Eigen;

// Determines Morten value according to z-curve.
inline 
uint64_t z_value(HashKey const& key)
{
    std::array<unsigned int, 3> x = {
        static_cast<int64_t>(key[0]) - (std::numeric_limits<int>::lowest() + 1),
        static_cast<int64_t>(key[1]) - (std::numeric_limits<int>::lowest() + 1),
        static_cast<int64_t>(key[2]) - (std::numeric_limits<int>::lowest() + 1)
    };

    return morton_lut(x);
}

CompactNSearch::CompactNSearch(Real r, bool erase_emtpy_cells)
    : m_r2(r * r), m_inv_cell_size(1.0 / r)
    , m_erase_empty_cells(erase_emtpy_cells)
    , m_initialized(false)
    , m_sorted(false)
    //, m_update_queue(0)
{
}

// Computes triple index to a world space position x.
inline HashKey
CompactNSearch::cell_index(Vector3d const& x)
{
    HashKey ret;
    for (unsigned int i(0); i < 3; ++i)
    {
        if (x[i] >= 0.0) ret[i] = static_cast<int>(m_inv_cell_size * x[i]);
        else ret[i] = static_cast<int>(m_inv_cell_size * x[i]) - 1;
    }
    return ret;
}

// Determines permutation table for particle array.
// If sort_boundary is true the boundary particles are sorted accordingly,
// but remain in their index range.
void
CompactNSearch::z_sort()
{
    // Z-sort of particles.
    for (auto& d : m_discretizations)
    {
        d.m_sort_table.resize(d.n_points());
        std::iota(d.m_sort_table.begin(), d.m_sort_table.end(), 0);

        std::sort(d.m_sort_table.begin(), d.m_sort_table.end(),
            [&](unsigned int a, unsigned int b)
        {
            return z_value(cell_index(d.point(a))) < z_value(cell_index(d.point(b)));
        });
    }
    m_sorted = true;
    m_initialized = false;
}


// Build hash table and entry array from scratch.
void
CompactNSearch::init()
{
    m_entries.clear();
    m_map.clear();

    // Determine existing entries.
    std::vector<HashKey> temp_keys;
    for (auto j(0); j < m_discretizations.size(); ++j)
    {
        auto& d = m_discretizations[j];
        for (unsigned int i = 0; i < d.n_points(); i++)
        {
            HashKey const& key = cell_index(d.point(i));
            d.m_keys[i] = d.m_old_keys[i] = key;
            auto it = m_map.find(key);
            if (it == m_map.end())
            {
                m_entries.push_back({{ j, i }});
                temp_keys.push_back(key);
                m_map[key] = static_cast<unsigned int>(m_entries.size() - 1);
            }
            else m_entries[it->second].add({j, i});
        }
    }

    // Z-sort cells.
    //std::sort(m_entries.begin(), m_entries.end(), 
    //[&](HashEntry const& a, HashEntry const& b)
    //{
    //	return a.z < b.z;
    //});
    insert_sort(temp_keys);
    m_map.clear();
    for (unsigned int i(0); i < m_entries.size(); ++i)
    {
        m_map.insert({ temp_keys[i], i });
    }

    m_sorted = false;
    m_initialized = true;
}

void
CompactNSearch::neighborhood_search()
{
    if (!m_initialized || m_sorted)
    {
        init();
        m_initialized = true;
    }

    // Precompute cell indices.
    for (auto& d : m_discretizations)
    {
        if (!d.is_moving()) continue;
        d.m_keys.swap(d.m_old_keys);
        for (unsigned int i(0); i < d.n_points(); ++i)
            d.m_keys[i] = cell_index(d.point(i));
    }

    update_hash_table_serial();
    if (m_erase_empty_cells) erase_empty_entries();
    query();
}

void
CompactNSearch::insert_sort(std::vector<HashKey>& keys)
{
    for (unsigned int i(1); i < m_entries.size(); ++i)
    for (unsigned int j(i); j > 0 && z_value(keys[j]) < 
        z_value(keys[j - 1]); --j)
    {
        std::swap(m_entries[j - 1], m_entries[j]);
        std::swap(keys[j - 1], keys[j]);
    }
}

void
CompactNSearch::erase_empty_entries()
{
    // Indicated empty cells.
    std::vector<unsigned int> deleted;
    for (int i(static_cast<int>(m_entries.size() - 1)); i >= 0; --i)
    {
        if (!m_entries[i].indices.empty())
            continue;

        deleted.push_back(i);
        m_entries.erase(m_entries.begin() + i);
    }

    // Erase empty cells and update indices from remaining cells.
    auto it = m_map.begin();
    while (it != m_map.end())
    {
        auto& kvp = *it;

        bool erased(false);
        for (unsigned int d : deleted)
        {
            if (kvp.second == d)
            {
                it = m_map.erase(it);
                erased = true;
                break;
            }
        }
        if (erased)
            continue;

        for (unsigned int i(0); i < deleted.size(); ++i)
        {
            if (kvp.second >= deleted[i])
            {
                kvp.second -= static_cast<unsigned int>(deleted.size() - i);
                break;
            }
        }
        ++it;
    }
}

//void
//CompactNSearch::update_hash_table_parallel()
//{
//    // Indicate particles changing inheriting cell.
//    concurrency::parallel_for_each(m_map.begin(), m_map.end(), 
//    [&](std::pair<HashKey, unsigned int> const& kvp)
//    {
//        HashKey const& old_key = kvp.first;
//        auto const& entry = m_entries[kvp.second];
//        auto const& indices = entry.indices;
//        for (unsigned int i_(0); i_ < entry.n_indices(); ++i_)
//        {
//            auto const& i = indices[i_];
//            auto const& d = m_discretizations[i.object_id];
//            if (!d.is_moving()) continue;
//            if (d.m_keys[i] != old_key)
//            {
//                m_update_queue.push(static_cast<unsigned int>(i));
//            }
//        }
//    }
//    );
//    //concurrency::parallel_for(std::size_t(0), std::size_t(m_n),
//    //	[&](std::size_t i)
//    //{
//    //	if (m_keys[i] != m_old_keys[i])
//    //		m_update_queue.push(static_cast<unsigned int>(i));
//    //}
//    //);
//
//    // Change move particles between cells.
//    while (!m_update_queue.empty())
//    {
//        unsigned int i;
//        m_update_queue.pop(i);
//
//        HashKey const& key = m_keys[i];
//        auto it = m_map.find(key);
//        if (it == m_map.end())
//        {
//            m_entries.push_back({ i });
//            m_map.insert({ key, static_cast<unsigned int>(m_entries.size() - 1) });
//        }
//        else
//        {
//            auto& entry = m_entries[it->second];
//            entry.add(i);
//        }
//
//        m_entries[m_map[m_old_keys[i]]].erase(i);
//    }
//}

void
CompactNSearch::update_hash_table_serial()
{
    // Indicate particles changing inheriting cell.
    for (auto j(0); j < m_discretizations.size(); ++j)
    {
        auto& d = m_discretizations[j];
        for (unsigned int i(0); i < d.n_points(); ++i)
        {
            if (d.m_keys[i] == d.m_old_keys[i]) continue;

            HashKey const& key = d.m_keys[i];
            auto it = m_map.find(key);
            if (it == m_map.end())
            {
                m_entries.push_back({{j, i}});
                m_map.insert({ key, static_cast<unsigned int>(m_entries.size() - 1) });
            }
            else
            {
                auto& entry = m_entries[it->second];
                entry.add({j, i});
            }

            m_entries[m_map[d.m_old_keys[i]]].erase({j, i});
        }
    }
}

void
CompactNSearch::query()
{
    for (auto& d : m_discretizations)
    if  (d.is_neighborsearch_enabled())
    for (auto& n : d.m_neighbors)
        n.clear();

    // Perform neighborhood search.
    concurrency::parallel_for_each(m_map.begin(), m_map.end(), 
    [&](std::pair<HashKey, unsigned int> kvp)
    {
        HashEntry const& entry = m_entries[kvp.second];
        HashKey const& key = kvp.first;

        for (char dj = -1; dj <= 1; dj++)
        for (char dk = -1; dk <= 1; dk++)
        for (char dl = -1; dl <= 1; dl++)
        {
            if (dj == 0 && dk == 0 && dl == 0)
            {
                for (unsigned int a(0); a < entry.n_indices(); ++a)
                for (unsigned int b(a + 1); b < entry.n_indices(); ++b)
                {
                    auto const& va = entry.indices[a];
                    auto const& vb = entry.indices[b];
                    auto& da = m_discretizations[va.object_id];
                    auto& db = m_discretizations[vb.object_id];
                    unsigned int ia = va.index;
                    unsigned int ib = vb.index;

                    if (!da.is_neighborsearch_enabled() && 
                        !db.is_neighborsearch_enabled()) continue;

                    if ((da.point(ia) - db.point(ib)).squaredNorm() < m_r2)
                    {
                        if (da.is_neighborsearch_enabled()) da.m_neighbors[ia].push_back(vb);
                        if (db.is_neighborsearch_enabled()) db.m_neighbors[ib].push_back(va);
                    }
                }
            }
            else
            {
                auto it = m_map.find({ key[0] + dj, key[1] + dk, key[2] + dl });
                if (it == m_map.end())
                    continue;

                for (unsigned int i(0); i < entry.n_indices(); ++i)
                {
                    auto const& va = entry.indices[i];
                    auto& da = m_discretizations[va.object_id];
                    if (!da.is_neighborsearch_enabled()) continue;
                    HashEntry const& entry_ = m_entries[it->second];
                    unsigned int n_ind = entry_.n_indices();
                    for (unsigned int j(0); j < n_ind; ++j)
                    {
                        auto const& vb = entry_.indices[j];
                        auto& db = m_discretizations[vb.object_id];
						if ((da.point(va.index) - db.point(vb.index)).squaredNorm() < m_r2)
                            da.m_neighbors[va.index].push_back(vb);
                    }
                }
            }
        }
    });
}



