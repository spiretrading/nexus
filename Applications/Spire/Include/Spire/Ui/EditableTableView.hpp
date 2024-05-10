#ifndef SPIRE_EDITABLE_TABLE_VIEW_HPP
#define SPIRE_EDITABLE_TABLE_VIEW_HPP
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a TableView that supports deleting rows and editing table cells.
   */
  class EditableTableView : public TableView {
    public:

      /**
       * Constructs an EditableTableView.
       * @param table The model of values to display.
       * @param header The model used to display the header.
       * @param table_filter The filter to apply to a column.
       * @param current The current value.
       * @param selection The selection.
       * @param item_builder The TableViewItemBuilder to use.
       * @param comparator The comparison function.
       * @param parent The parent widget.
       */
      EditableTableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<HeaderModel> header,
        std::shared_ptr<TableFilter> table_filter,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        TableViewItemBuilder item_builder, Comparator comparator,
        QWidget* parent = nullptr);

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      bool m_is_processing_key;

      QWidget* make_table_item(TableViewItemBuilder& item_builder,
        const std::shared_ptr<TableModel>& table, int row, int column);
      void delete_row(const TableRowIndexTracker& row);
      bool navigate_next();
      bool navigate_previous();
  };
}

#endif
