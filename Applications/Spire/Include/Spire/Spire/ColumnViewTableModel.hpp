#ifndef SPIRE_COLUMN_VIEW_TABLE_MODEL_HPP
#define SPIRE_COLUMN_VIEW_TABLE_MODEL_HPP
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements a TableModel that provides a view into a single column of
   * a source table model.
   */
  class ColumnViewTableModel : public TableModel {
    public:

      /**
       * Constructs a ColumnViewTableModel from a specified column of
       * the table model.
       * @param source The table model.
       * @param column The index of the column to be viewed. 
       */
      ColumnViewTableModel(std::shared_ptr<TableModel> source, int column);

      int get_row_size() const override;
      int get_column_size() const override;
      AnyRef at(int row, int column) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      int m_column;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
