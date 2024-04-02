#ifndef SPIRE_TABLE_ROW_INDEX_TRACKER_HPP
#define SPIRE_TABLE_ROW_INDEX_TRACKER_HPP
#include <boost/signals2/connection.hpp>
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
       * @param table The table to monitor the row for.
       * @param index The initial index of the row to track.
       */
      TableRowIndexTracker(const TableModel& table, int index);

      /**
       * Returns the current index of the row being tracked, if the row was
       * deleted then the index will be <i>-1</i>.
       */
      int get_index() const;

    private:
      boost::signals2::scoped_connection m_connection;
      int m_index;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
