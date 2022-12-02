#ifndef SPIRE_ORDER_TASKS_TABLE_VIEW_MODEL_HPP
#define SPIRE_ORDER_TASKS_TABLE_VIEW_MODEL_HPP
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements an OrderTasksTableViewModel that represents the table of
   * the TableView inside the OrderTasksPage. Its first column and last row
   * are empty.
   */
  class OrderTasksTableViewModel : public TableModel {
    public:

      /**
       * Constructs an OrderTasksTableViewModel.
       * @param source The source model.
       */
      explicit OrderTasksTableViewModel(std::shared_ptr<TableModel> source);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State set(
        int row, int column, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
