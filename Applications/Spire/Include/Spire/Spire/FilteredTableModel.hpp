#ifndef SPIRE_FILTERED_TABLE_MODEL_HPP
#define SPIRE_FILTERED_TABLE_MODEL_HPP
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"
#include "Spire/Spire/TableToListModel.hpp"

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

      /**
       * Maps a row index from this table into the source table.
       * @param index An index into a row of this table.
       * @return The corresponding row index into the source table or
       *         <code>-1</code> iff the row index is not valid.
       */
      int index_to_source(int index) const;

      /**
       * Maps a row index from the source table to this table.
       * @param index An index into a row of the source table.
       * @return The corresponding row index into this table, or <code>-1</code>
       *         iff the row index is not valid.
       */
      int index_from_source(int index) const;

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State set(
        int row, int column, const std::any& value) override;

      QValidator::State remove(int row) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      std::shared_ptr<TableToListModel> m_list;
      FilteredListModel<RowView> m_filtered_list;
      boost::signals2::scoped_connection m_connection;
      TableModelTransactionLog m_transaction;

      void on_operation(const ListModel<RowView>::Operation& operation);
  };
}

#endif
