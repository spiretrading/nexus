#ifndef SPIRE_COLUMN_VIEW_LIST_MODEL_HPP
#define SPIRE_COLUMN_VIEW_LIST_MODEL_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/ListModelTransactionLog.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ListModel that provides a view into a single column of
   * a source table model.
   */
  class ColumnViewListModel : public ListModel {
    public:

      /** 
       * Constructs a ColumnViewListModel from a specified column of
       * the table model.
       * @param source The table model.
       * @param column The index of the column to be viewed. 
       */
      ColumnViewListModel(std::shared_ptr<TableModel> source, int column);

      int get_size() const override;

      const std::any& at(int index) const override;

      QValidator::State set(int index, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      int m_column;
      ListModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
