#ifndef SPIRE_TABLE_HEADER_HPP
#define SPIRE_TABLE_HEADER_HPP
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a list of TableHeaderItems that make up a TableView's header. */
  class TableHeader : public QWidget {
    public:

      /**
       * Signals an action to change a column's sort order.
       * @param column The index of the column that triggered the action.
       * @param order The sort order to update to.
       */
      using SortSignal =
        Signal<void (int column, TableHeaderItem::Order order)>;

      /**
       * Signals an action to change a column's filter.
       * @param column The index of the column that triggered the action.
       */
      using FilterSignal = Signal<void (int column)>;

      /**
       * Constructs a TableHeader.
       * @param items The list of items in order from left-to-right.
       * @param parent The parent widget.
       */
      explicit TableHeader(
        std::shared_ptr<ListModel<TableHeaderItem::Model>> items,
        QWidget* parent = nullptr);

      /** Returns the list of items represented by this header. */
      const std::shared_ptr<ListModel<TableHeaderItem::Model>>& get_items()
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
      std::shared_ptr<ListModel<TableHeaderItem::Model>> m_items;
      boost::signals2::scoped_connection m_items_connection;

      void on_items_update(
        const ListModel<TableHeaderItem::Model>::Operation& operation);
  };
}

#endif
