#ifndef SPIRE_ORDER_TASKS_ROW_HPP
#define SPIRE_ORDER_TASKS_ROW_HPP
#include <QPointer>
#include "Spire/KeyBindings/OrderTask.hpp"
#include "Spire/KeyBindings/TableRow.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/HoverObserver.hpp"

namespace Spire {
namespace Styles {

  /** Selects the item being edited. */
  using Editing = StateSelector<void, struct EditingSelectorTag>;

  /** Selects the grab handle being hovered. */
  using HoveredGrabHandle =
    StateSelector<void, struct HoveredGrabHandleSelectorTag>;

  /** Selects the row which is out of range. */
  using OutOfRangeRow = StateSelector<void, struct OutOfRangeRowSelectorTag>;
}

  /** Represents a row of the TableView within the OrderTasksPage. */
  class OrderTasksRow : public TableRow {
    public:

      /** Represents a cell of the TableView. */
      struct TableCell {

        /** The cell component. */
        QWidget* m_cell;

        /** The editor of this cell which could be null. */
        EditableBox* m_editor;
      };

      /** Lists out the columns of the row. */
      enum class Column {

        /** The grab handle field. */
        GRAB_HANDLE,

        /** The name field. */
        NAME,

        /** The region field. */
        REGION,

        /** The destination field. */
        DESTINATION,

        /** The order type field. */
        ORDER_TYPE,

        /** The side field. */
        SIDE,

        /** The quantity field. */
        QUANTITY,

        /** The time in force field. */
        TIME_IN_FORCE,

        /** The key field. */
        KEY
      };

      /** The number of columns in this row. */
      static const auto COLUMN_SIZE = 9;

      /**
       * Constructs an OrderTasksRow.
       * @param order_tasks The list of OrderTasks.
       * @param row The row index of this OrderTasksRow.
       */
      OrderTasksRow(std::shared_ptr<ListModel<OrderTask>> order_tasks, int row);

      int get_row_index() const override;

      QWidget* get_row() const override;

      QWidget* get_grab_handle() const override;

      bool is_draggable() const override;

      void set_draggable(bool is_draggable) override;

      bool is_ignore_filters() const override;

      void set_ignore_filters(bool is_ignore_filters) override;

      bool is_out_of_range() const override;

      void set_out_of_range(bool is_out_of_range) override;

      /**
       * Build a cell.
       * @param region_query_model The model used to query region matches.
       * @param destinations The destination database.
       * @param markets The market database.
       * @param model The table model of the TableView.
       * @param row The row index.
       * @param column The column index.
       */
      TableCell build_cell(
        const std::shared_ptr<ComboBox::QueryModel>& region_query_model,
        const Nexus::DestinationDatabase& destinations,
        const Nexus::MarketDatabase& markets,
        const std::shared_ptr<TableModel>& table, int row, int column);

    private:
      std::shared_ptr<ListModel<OrderTask>> m_order_tasks;
      int m_row_index;
      QPointer<QWidget> m_row;
      QPointer<QWidget> m_grab_handle;
      bool m_is_draggable;
      bool m_is_ignore_filters;
      bool m_is_out_of_range;
      std::unique_ptr<HoverObserver> m_hover_observer;
      boost::signals2::scoped_connection m_operation_connection;

      void make_hover_observer();
      EditableBox* make_editor(
        const std::shared_ptr<ComboBox::QueryModel>& region_query_model,
        const Nexus::DestinationDatabase& destinations,
        const Nexus::MarketDatabase& markets,
        const std::shared_ptr<TableModel>& table, int row, int column);
      EditableBox* make_empty_editor(int column,
        const std::shared_ptr<ComboBox::QueryModel>& region_query_model);
      void on_operation(const ListModel<OrderTask>::Operation& operation);
      void on_submit(AnyInputBox* input_box, Column column,
        const AnyRef& submission);
  };
}

#endif
