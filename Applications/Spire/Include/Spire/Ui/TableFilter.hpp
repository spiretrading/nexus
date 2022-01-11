#ifndef SPIRE_TABLE_FILTER_HPP
#define SPIRE_TABLE_FILTER_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Base class used to filter rows from a TableView. */
  class TableFilter {
    public:

      /** Specifies how this filter is applied to a column. */
      enum class Filter {

        /** The column can not be filtered. */
        NONE,

        /** The column has a filter applied. */
        FILTERED,

        /** The column does not have a filter applied. */
        UNFILTERED
      };

      /**
       * Signals a change in a column's filter.
       * @param column The column whose filter changed.
       * @param filter The state of the <i>column</i>'s filter.
       */
      using FilterSignal = Signal<void (int column, Filter filter)>;

      virtual ~TableFilter() = default;

      /**
       * Returns how this filter is applied to a given column.
       * @param column The index of a column.
       * @return How this filter is applied to the <code>column</code>.
       */
      virtual Filter get_filter(int column) = 0;

      /**
       * Returns the widget used to display the filtering options for a
       * given column.
       * @param column The column whose filtering options are to be
       *        displayed.
       * @param parent The parent widget.
       */
      virtual QWidget* make_filter_widget(int column, QWidget& parent) = 0;

      /**
       * Returns <code>true</code> iff a given row within a table model
       * needs to be filtered.
       * @param model The model to filter.
       * @param row The index of the row within the model to filter.
       * @return <code>true</code> iff the <i>row</i> within the
       *         <i>model</i> is filtered out of the TableView.
       */
      virtual bool is_filtered(const TableModel& model, int row) const = 0;

      /** Connects a slot to the FilterSignal. */
      virtual boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const = 0;
  };
}

#endif
