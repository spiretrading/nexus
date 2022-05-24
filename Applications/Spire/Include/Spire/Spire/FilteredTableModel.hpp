#ifndef SPIRE_FILTERED_TABLE_MODEL_HPP
#define SPIRE_FILTERED_TABLE_MODEL_HPP
#include <functional>
#include <tuple>
#include <vector>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements a TableModel that provides a view over a source model by
   * filtering out some rows.
   */
  class FilteredTableModel : public TableModel {
    public:

      /**
       * The type of callable used to filter rows.
       * @param model The model being filtered.
       * @param row The index of the row within the <i>model</i> to test.
       * @return <code>true</code> iff the <i>row</i> should be excluded.
       */
      using Filter = std::function<bool (const TableModel& model, int row)>;

      /**
       * Constructs a FilteredTableModel from a TableModel and a filter
       * function.
       * @param source The model to filter.
       * @param filter The filter applied to every row of the <i>source</i>.
       */
      FilteredTableModel(std::shared_ptr<TableModel> source, Filter filter);

      /** Applies a new filter to this model. */
      void set_filter(const Filter& filter);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State set(
        int row, int column, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      Filter m_filter;
      std::vector<int> m_filtered_data;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      std::tuple<bool, std::vector<int>::iterator> find(int index);
      void on_operation(const Operation& operation);
  };
}

#endif
