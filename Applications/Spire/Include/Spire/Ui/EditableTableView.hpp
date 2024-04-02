#ifndef SPIRE_EDITABLE_TABLE_VIEW_HPP
#define SPIRE_EDITABLE_TABLE_VIEW_HPP
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Represents an editable TableView. */
  class EditableTableView : public TableView {
    public:

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

    private:
      QWidget* make_table_item(const ViewBuilder& view_builder,
        const std::shared_ptr<TableModel>& table, int row, int column);
      void delete_row(const TableRowIndexTracker& row);
  };
}

#endif
