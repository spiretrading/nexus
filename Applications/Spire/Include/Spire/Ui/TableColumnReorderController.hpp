#ifndef SPIRE_TABLE_COLUMN_REORDER_CONTROLLER_HPP
#define SPIRE_TABLE_COLUMN_REORDER_CONTROLLER_HPP
#include <vector>
#include <QMouseEvent>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Implements a controller for reordering table columns via drag and drop. */
  class TableColumnReorderController : public QObject {
    public:

      /**
       * Signals the column has being moved from source to destination.
       * @param source The index of the column to move.
       * @param destination The index to move the column to.
       */
      using ColumnMoved = Signal<void(int source, int destination)>;

      /**
       * Constructs TableColumnReorderController with the default column order.
       * @param table_view The TableView that the controller works on.
       * @param item_builder The TableViewItemBuilder to use.
       */
      TableColumnReorderController(TableView& table_view,
        TableViewItemBuilder item_builder);

      /**
       * Constructs a TableColumnReorderController.
       * @param table_view The TableView that the controller works on.
       * @param item_builder The TableViewItemBuilder to use.
       * @param column_order The column order to use.
       */
      TableColumnReorderController(TableView& table_view,
        TableViewItemBuilder item_builder, std::vector<int> column_order);

      /** Connects a slot to the ColumnMoved signal. */
      boost::signals2::connection connect_column_moved_signal(
        const ColumnMoved::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable ColumnMoved m_column_moved_signal;
      TableView* m_table_view;
      TableViewItemBuilder m_item_builder;
      std::vector<int> m_column_order;
      QWidget* m_column_preview;
      QWidget* m_column_cover;
      QWidget* m_horizontal_scroll_bar_parent;
      QWidget* m_vertical_scroll_bar_parent;
      int m_last_mouse_x;
      int m_left_padding;
      int m_source_index;
      int m_current_index;
      int m_preview_x_offset;
      std::vector<int> m_widths;
      boost::signals2::scoped_connection m_connection;

      void start_drag(const QMouseEvent& mouse_event);
      void stop_drag();
      void drag_move(const QMouseEvent& mouse_event);
      void move_column_cover(int index);
      void update_horizontal_scroll_position(int mouse_x);
      void on_operation(
        const ListModel<TableHeaderItem::Model>::Operation& operation);
  };
}

#endif
