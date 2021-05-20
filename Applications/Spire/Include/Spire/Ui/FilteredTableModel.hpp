#ifndef SPIRE_FILTERED_TABLE_MODEL_HPP
#define SPIRE_FILTERED_TABLE_MODEL_HPP
#include <functional>
#include <tuple>
#include <vector>
#include "Spire/Ui/ModelTransactionLog.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a TableModel that provides a view over a source model by
   * filtering out some rows.
   */
  class FilteredTableModel : public TableModel {
    public:

      /** 
       * Constructs a FilteredTableModel from a TableModel and a filter function.
       * @param source The model.
       * @param filter A function that takes a TableModel and the index of a
       *               row, and returns true if that row should be excluded of
       *               the model.
       */
      FilteredTableModel(std::shared_ptr<TableModel> source,
        std::function<bool(const TableModel&, int)> filter);

      int get_row_size() const override;

      int get_column_size() const override;

      const std::any& at(int row, int column) const override;

      QValidator::State set(int row, int column, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      std::function<bool(const TableModel&, int)> m_filter;
      std::vector<int> m_filtered_data;
      ModelTransactionLog<TableModel> m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      std::tuple<bool, std::vector<int>::iterator> find(int index);
      void on_operation(const Operation& operation);
  };
}

#endif
