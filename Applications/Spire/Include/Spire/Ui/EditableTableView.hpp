#ifndef SPIRE_EDITABLE_TABLE_VIEW_HPP
#define SPIRE_EDITABLE_TABLE_VIEW_HPP
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Represents an editable TableView. */
  class EditableTableView : public TableView {
    public:

      /**
       * Signals that the row is being clicked to delete.
       */
      using DeleteSignal = Signal<void (int row)>;

      /**
       * Constructs an EditableTableView.
       * @param table The model of values to display.
       * @param header The model used to display the header.
       * @param table_filter The filter to apply to a column.
       * @param current The current value.
       * @param selection The selection.
       * @param view_builder The ViewBuilder to use.
       * @param comparator The comparison function.
       * @param parent The parent widget.
       */
      EditableTableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<HeaderModel> header,
        std::shared_ptr<TableFilter> table_filter,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
        Comparator comparator, QWidget* parent = nullptr);

      /** Connects a slot to the DeleteSignal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;

    private:
      mutable DeleteSignal m_delete_signal;
      TableBody* m_table_body;
      ArrayListModel<QWidget*> m_rows;
      boost::signals2::scoped_connection m_operation_connection;

      QWidget* make_table_item(ViewBuilder source_view_builder,
        const std::shared_ptr<TableModel>& table, int row, int column);
      void delete_current_row();
      void on_source_table_operation(const TableModel::Operation& operation);
  };
}

#endif
