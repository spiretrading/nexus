#ifndef SPIRE_TABLE_CURRENT_CONTROLLER_HPP
#define SPIRE_TABLE_CURRENT_CONTROLLER_HPP
#include <memory>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/TableIndex.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Implements a controller managing a TableView's current index. */
  class TableCurrentController {
    public:
      using Index = TableIndex;

      /** The type of model representing the index of the current value. */
      using CurrentModel = ValueModel<boost::optional<Index>>;

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

    private:
      std::shared_ptr<CurrentModel> m_current;
      int m_row_size;
      int m_column_size;
  };
}

#endif
