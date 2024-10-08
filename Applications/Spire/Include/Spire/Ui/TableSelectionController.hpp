#ifndef SPIRE_TABLE_SELECTION_CONTROLLER_HPP
#define SPIRE_TABLE_SELECTION_CONTROLLER_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableIndex.hpp"
#include "Spire/Ui/TableSelectionModel.hpp"

namespace Spire {

  /** Implements a controller managing a TableView's selection. */
  class TableSelectionController {
    public:
      using Index = TableIndex;

      /** The type of model representing the table selection. */
      using SelectionModel = TableSelectionModel;

      /** Enumerates the different ways that items can be selected. */
      enum class Mode {

        /** Only a single item is selected. */
        SINGLE,

        /** Items are selected one by one. */
        INCREMENTAL,

        /** A range of items are selected at a time. */
        RANGE
      };

      /**
       * Constructs a TableSelectionController over a given selection model with
       * an initial selection mode set to SINGLE.
       * @param selection The model of selected items.
       * @param row_size The number of rows in the table.
       * @param column_size The number of columns in the table.
       */
      explicit TableSelectionController(
        std::shared_ptr<SelectionModel> selection, int row_size,
        int column_size);

      /** Returns the selection model. */
      const std::shared_ptr<SelectionModel>& get_selection() const;

      /** Returns the selection mode. */
      Mode get_mode() const;

      /** Sets the selection mode. */
      void set_mode(Mode mode);

      /** Returns the number of rows. */
      int get_row_size() const;

      /** Returns the number of columns. */
      int get_column_size() const;

      /**
       * Updates this controller with a new row.
       * @param index The index of the row.
       */
      void add_row(int index);

      /**
       * Updates this controller with a removed row.
       * @param index The index of the removed row.
       */
      void remove_row(int index);

      /**
       * Updates this controller when a row is moved.
       * @param source The index where the row was moved from.
       * @param destination The index that the row was moved to.
       */
      void move_row(int source, int destination);

      /** Selects all items. */
      void select_all();

      /**
       * Updates the selection when an item is clicked.
       * @param index The index of the clicked item.
       */
      void click(Index index);

      /**
       * Updates the selection when an item is navigated to (by keyboard).
       * @param index The index to navigate to.
       */
      void navigate(Index index);

      /**
       * Connects a slot to a signal indicating a change to the item selection.
       */
      boost::signals2::connection connect_item_operation_signal(
        const ListModel<Index>::OperationSignal::slot_type& slot) const;

      /**
       * Connects a slot to a signal indicating a change to the row selection.
       */
      boost::signals2::connection connect_row_operation_signal(
        const ListModel<int>::OperationSignal::slot_type& slot) const;

      /**
       * Connects a slot to a signal indicating a change to the column
       * selection.
       */
      boost::signals2::connection connect_column_operation_signal(
        const ListModel<int>::OperationSignal::slot_type& slot) const;

    private:
      mutable ListModel<Index>::OperationSignal m_item_operation_signal;
      mutable ListModel<int>::OperationSignal m_row_operation_signal;
      mutable ListModel<int>::OperationSignal m_column_operation_signal;
      Mode m_mode;
      std::shared_ptr<SelectionModel> m_selection;
      int m_row_size;
      int m_column_size;
      boost::optional<Index> m_current;
      boost::optional<Index> m_range_anchor;
      boost::signals2::scoped_connection m_item_connection;
      boost::signals2::scoped_connection m_row_connection;
      boost::signals2::scoped_connection m_column_connection;

      void on_item_operation(const ListModel<Index>::Operation& operation);
      void on_row_operation(const ListModel<int>::Operation& operation);
      void on_column_operation(const ListModel<int>::Operation& operation);
  };
}

#endif
