#ifndef SPIRE_TABLE_ROW_DRAG_DROP_HPP
#define SPIRE_TABLE_ROW_DRAG_DROP_HPP
#include "Spire/KeyBindings/TableRow.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects the current item being dragged. */
  using DraggingCurrent =
    StateSelector<void, struct DraggingCurrentSelectorTag>;

  /** Selects the row to be dropped above the row being dragged. */
  using DropUpIndicator =
    StateSelector<void, struct DropUpIndicatorSelectorTag>;

  /** Selects the row to be dropped below the row being dragged. */
  using DropDownIndicator =
    StateSelector<void, struct DropDownIndicatorSelectorTag>;
}

  /** Implements row drag and drop of a table view. */
  class TableRowDragDrop : public QObject {
    public:

      /**
       * Constructs a TableRowDragDrop.
       * @param model The list model used to move rows.
       * @param rows The rows of the table.
       * @param table_view The table view that performs row drag and drop.
       */
      TableRowDragDrop(std::shared_ptr<AnyListModel> model,
        std::shared_ptr<ListModel<std::shared_ptr<TableRow>>> rows,
        TableView& table_view);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      class PreviewRow;
      class PaddingRow;
      std::shared_ptr<AnyListModel> m_model;
      std::shared_ptr<ListModel<std::shared_ptr<TableRow>>> m_rows;
      TableView* m_table_view;
      TableHeader* m_table_header;
      ScrollBox* m_scroll_box;
      TableBody* m_table_body;
      std::shared_ptr<SortedTableModel> m_sorted_model;
      PreviewRow* m_preview_row;
      PaddingRow* m_padding_row;
      QWidget* m_grab_handle;
      boost::optional<QPoint> m_press_pos;
      boost::optional<int> m_drag_index;
      bool m_is_sorted;
      int m_row_count;
      boost::signals2::scoped_connection m_operation_connection;

      void move(int source, int destination);
      void clear_sort_order();
      void drag_start(const QPoint& pos);
      void drag_end();
      void on_operation(const TableModel::Operation& operation);
      void on_sort(int index, TableHeaderItem::Order order);
  };
}

#endif
