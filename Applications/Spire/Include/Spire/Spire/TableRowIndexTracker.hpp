#ifndef SPIRE_TABLE_ROW_INDEX_TRACKER_HPP
#define SPIRE_TABLE_ROW_INDEX_TRACKER_HPP
#include "Spire/Spire/ListIndexTracker.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /**
   * Keeps track of the index of a row within a table in the face of table
   * updates such as row deletions, moves, and additions.
   */
  class TableRowIndexTracker {
    public:

      /**
       * Constructs a TableRowIndexTracker.
       * @param index The initial index of the row to track.
       */
      explicit TableRowIndexTracker(int index);

      /**
       * Returns the current index of the row being tracked, if the row was
       * deleted then the index will be <i>-1</i>.
       */
      int get_index() const;

      /** Sets the index of the row to track. */
      void set(int index);

      /**
       * Updates the row index based on an operation.
       * @param operation The operation performed on the table.
       */
      void update(const TableModel::Operation& operation);

    private:
      ListIndexTracker m_tracker;
  };
}

#endif
