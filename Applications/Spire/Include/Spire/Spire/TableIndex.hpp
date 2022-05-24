#ifndef SPIRE_TABLE_INDEX_HPP
#define SPIRE_TABLE_INDEX_HPP
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

#endif
