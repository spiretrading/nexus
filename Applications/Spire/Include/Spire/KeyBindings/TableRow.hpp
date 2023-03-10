#ifndef SPIRE_TABLE_ROW_HPP
#define SPIRE_TABLE_ROW_HPP
#include <QWidget>

namespace Spire {

  /** Represents a row of the TableView. */
  class TableRow {
    public:

      virtual ~TableRow() = default;

      /** Returns the row index. */
      virtual int get_row_index() const = 0;

      /** Returns the row widget. */
      virtual QWidget* get_row() const = 0;

      /** Returns the drag widget. */
      virtual QWidget* get_grab_handle() const = 0;

      /** Returns <code>true</code> iff this row is draggable. */
      virtual bool is_draggable() const = 0;

      /** Sets whether this row is draggable. */
      virtual void set_draggable(bool is_draggable) = 0;

      /** Returns <code>true</code> iff this row is excluded from filtering. */
      virtual bool is_ignore_filters() const = 0;

      /** Sets whether this row is excluded from filtering. */
      virtual void set_ignore_filters(bool is_ignore_filters) = 0;

      /**
       * Returns <code>true</code> iff this row does not meet the filter or
       * sort criteria set.
       */
      virtual bool is_out_of_range() const = 0;

      /** Sets whether this row doesn't meet the filter or sort criteria set. */
      virtual void set_out_of_range(bool is_out_of_range) = 0;

    protected:

      /** Constructs a table row. */
      TableRow() = default;

    private:
      TableRow(const TableRow&) = delete;
      TableRow& operator =(const TableRow&) = delete;
  };
}

#endif
