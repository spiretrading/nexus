#ifndef SPIRE_ROW_VIEW_LIST_MODEL_HPP
#define SPIRE_ROW_VIEW_LIST_MODEL_HPP
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include "Spire/Ui/ModelTransactionLog.hpp"
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ListModel that provides a view into a single row of a source
   * table model.
   */
  class RowViewListModel : public ListModel {
    public:

      /** 
       * Constructs a RowViewListModel from a specified row of the table model.
       * @param source The table model.
       * @param row The index of the row to be viewed. 
       */
      RowViewListModel(std::shared_ptr<TableModel> source, int row);

      int get_size() const override;

      const std::any& at(int index) const override;

      QValidator::State set(int index, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      boost::optional<int> m_row;
      ModelTransactionLog<ListModel> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
