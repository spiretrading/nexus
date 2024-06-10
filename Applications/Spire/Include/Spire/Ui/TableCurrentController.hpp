#ifndef SPIRE_TABLE_CURRENT_CONTROLLER_HPP
#define SPIRE_TABLE_CURRENT_CONTROLLER_HPP
#include <memory>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/TableIndex.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a controller managing a TableView's current index. */
  class TableCurrentController {
    public:
      using Index = TableIndex;

      /** The type of model representing the index of the current value. */
      using CurrentModel = ValueModel<boost::optional<Index>>;

      /**
       * Signals a change to the current navigation index.
       * @param previous The index of the previous navigation index.
       * @param current The index of the current navigation index.
       */
      using UpdateSignal = Signal<void (
        const boost::optional<TableIndex>& previous,
        const boost::optional<TableIndex>& current)>;

      /**
       * Constructs a TableCurrentController over a given current index model.
       * @param current The current index model.
       * @param row_size The number of rows in the table.
       * @param column_size The number of columns in the table.
       */
      TableCurrentController(
        std::shared_ptr<CurrentModel> current, int row_size, int column_size);

      /** Returns the current index model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the current index. */
      const boost::optional<Index>& get() const;

      /** Returns the current row. */
      boost::optional<int> get_row() const;

      /** Returns the current column. */
      boost::optional<int> get_column() const;

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

      /** Sets the current to the first selectable item. */
      void navigate_home();

      /** Sets the current to the first row (column is unchanged). */
      void navigate_home_row();

      /** Sets the current to the first column (row is unchanged). */
      void navigate_home_column();

      /** Sets the current to the last selectable item. */
      void navigate_end();

      /** Sets the current to the last row (column is unchanged). */
      void navigate_end_row();

      /** Sets the current to the last column (row is unchanged). */
      void navigate_end_column();

      /** Sets the current to the next (or first) row. */
      void navigate_next_row();

      /** Sets the current to the next (or first) column. */
      void navigate_next_column();

      /** Sets the current to the previous (or first) row. */
      void navigate_previous_row();

      /** Sets the current to the previous (or first) column. */
      void navigate_previous_column();

      /**
       * Connects a slot to a signal indicating a change to the current index.
       */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<CurrentModel> m_current;
      int m_row_size;
      int m_column_size;
      boost::optional<Index> m_last_current;
      boost::signals2::scoped_connection m_connection;

      void on_current(const boost::optional<Index>& current);
  };
}

#endif
