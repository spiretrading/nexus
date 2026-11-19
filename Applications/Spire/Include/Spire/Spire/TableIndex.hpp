#ifndef SPIRE_TABLE_INDEX_HPP
#define SPIRE_TABLE_INDEX_HPP
#include <functional>
#include <boost/container_hash/hash.hpp>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Stores an index into a TableModel. */
  struct TableIndex {

    /** The index's row. */
    int m_row;

    /** The index's column. */
    int m_column;

    bool operator ==(const TableIndex&) const = default;
  };
}

namespace std {
  template<>
  struct hash<Spire::TableIndex> {
    size_t operator ()(const Spire::TableIndex& index) const noexcept {
      auto seed = hash<int>()(index.m_row);
      boost::hash_combine(seed, index.m_column);
      return seed;
    }
  };
}

#endif
