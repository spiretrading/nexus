#ifndef SPIRE_TABLE_HEADER_HPP
#define SPIRE_TABLE_HEADER_HPP
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"

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
       * Signals that a column's filter panel was opened or closed.
       * @param column The index of the column whose filter panel changed.
       * @param is_open <code>true</code> iff the filter panel is open.
       */
      using FilterOpenSignal = Signal<void (int column, bool is_open)>;

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

      /** Returns the list of widths of each item. */
      const std::shared_ptr<ListModel<int>>& get_widths() const;

      /** Returns a column's header item. */
      TableHeaderItem* get_item(int column);

      /** Returns the index of the given header item. */
      boost::optional<int> get_index(TableHeaderItem* item) const;

      /**
       * Connects a slot to the SortSignal.
       * @param slot The slot to connect.
       * @return A connection to the SortSignal.
       */
      boost::signals2::connection connect_sort_signal(
        const SortSignal::slot_type& slot) const;

      /**
       * Connects a slot to the FilterOpenSignal.
       * @param slot The slot to connect.
       * @return A connection to the FilterOpenSignal.
       */
      boost::signals2::connection connect_filter_open_signal(
        const FilterOpenSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable SortSignal m_sort_signal;
      mutable FilterOpenSignal m_filter_signal;
      std::shared_ptr<ListModel<TableHeaderItem::Model>> m_items;
      std::shared_ptr<ListModel<int>> m_widths;
      std::vector<TableHeaderItem*> m_item_views;
      boost::signals2::scoped_connection m_widths_connection;

      void on_widths_operation(const ListModel<int>::Operation& operation);
      void on_sort(int index, TableHeaderItem::Order order);
      void on_filter_open(int index, bool is_open);
  };
}

#endif
