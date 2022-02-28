#ifndef SPIRE_TABLE_OPERATION_LOG_HPP
#define SPIRE_TABLE_OPERATION_LOG_HPP
#include "Spire/Ui/TableModel.hpp"

namespace Spire {

  /** Logs operations performed on a TableModel. */
  class TableOperationLog {
    public:

      /**
       * Constructs a TableOperationLog that logs operations on the given
       * TableModel.
       * @param model The TableModel to log the operations of.
       */
      explicit TableOperationLog(std::shared_ptr<TableModel> model);

      /** Returns the total number of operations performed on the TableModel. */
      int operation_count() const;

      /** Returns the number of add operations. */
      int add_count() const;

      /** Returns the add operations, sorted sequentially. */
      const std::vector<TableModel::AddOperation>& adds() const;

      /** Returns the number of remove operations. */
      int remove_count() const;

      /** Returns the remove operations, sorted sequentially. */
      const std::vector<TableModel::RemoveOperation>& removes() const;

      /** Returns the number of move operations. */
      int move_count() const;
    
      /** Returns the move operations, sorted sequentially. */
      const std::vector<TableModel::MoveOperation>& moves() const;

      /** Returns the number of update operations. */
      int update_count() const;

      /** Returns the updates performed, sorted sequentially. */
      const std::vector<TableModel::UpdateOperation>& updates() const;

    private:
      boost::signals2::scoped_connection m_operation_connection;
      std::vector<TableModel::AddOperation> m_adds;
      std::vector<TableModel::RemoveOperation> m_removes;
      std::vector<TableModel::MoveOperation> m_moves;
      std::vector<TableModel::UpdateOperation> m_updates;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
