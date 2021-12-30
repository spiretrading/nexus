#ifndef SPIRE_TABLE_HEADER_HPP
#define SPIRE_TABLE_HEADER_HPP
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TableHeaderCell.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a list of TableHeaderCells that make up a TableView's header. */
  class TableHeader : public QWidget {
    public:

      /**
       * Signals an action to change a column's sort order.
       * @param column The index of the column that triggered the action.
       * @param order The sort order to update to.
       */
      using SortSignal =
        Signal<void (int column, TableHeaderCell::Order order)>;

      /**
       * Signals an action to change a column's filter.
       * @param column The index of the column that triggered the action.
       */
      using FilterSignal = Signal<void (int column)>;

      /**
       * Constructs a TableHeader.
       * @param cells The list of cells in order from left-to-right.
       * @param parent The parent widget.
       */
      explicit TableHeader(
        std::shared_ptr<ListModel<TableHeaderCell::Model>> cells,
        QWidget* parent = nullptr);

      /** Returns the list of cells represented by this header. */
      const std::shared_ptr<ListModel<TableHeaderCell::Model>>& get_cells()
        const;

      /**
       * Connects a slot to the SortSignal.
       * @param slot The slot to connect.
       * @return A connection to the SortSignal.
       */
      boost::signals2::connection connect_sort_signal(
        const SortSignal::slot_type& slot) const;

      /**
       * Connects a slot to the FilterSignal.
       * @param slot The slot to connect.
       * @return A connection to the FilterSignal.
       */
      boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const;

    private:
      mutable SortSignal m_sort_signal;
      mutable FilterSignal m_filter_signal;
      std::shared_ptr<ListModel<TableHeaderCell::Model>> m_cells;
      boost::signals2::scoped_connection m_cells_connection;

      void on_cells_update(
        const ListModel<TableHeaderCell::Model>::Operation& operation);
  };
}

#endif
