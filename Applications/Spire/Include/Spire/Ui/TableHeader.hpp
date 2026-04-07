#ifndef SPIRE_TABLE_HEADER_HPP
#define SPIRE_TABLE_HEADER_HPP
#include <QPointer>
#include <QVariantAnimation>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TranslatedListModel.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"

namespace Spire {
  class PopupBox;
  class ScrollBox;

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
       * Constructs a TableHeader.
       * @param items The list of items in order from left-to-right.
       * @param filter The filter used to create filter panels.
       * @param parent The parent widget.
       */
      TableHeader(std::shared_ptr<ListModel<TableHeaderItem::Model>> items,
        std::shared_ptr<TableFilter> filter, QWidget* parent = nullptr);

      /** Returns the list of items represented by this header. */
      const std::shared_ptr<ListModel<TableHeaderItem::Model>>& get_items()
        const;

      /** Returns the filter used to create filter panels. */
      const std::shared_ptr<TableFilter>& get_filter() const;

      /** Returns the list of widths of each item. */
      const std::shared_ptr<ListModel<int>>& get_widths() const;

      /** Returns a column's header item. */
      TableHeaderItem* get_item(int column);

      /** Returns the ScrollBox. */
      ScrollBox& get_scroll_box();

      /** Returns the index of the given header item. */
      boost::optional<int> get_index(TableHeaderItem* item) const;

      /**
       * Connects a slot to the SortSignal.
       * @param slot The slot to connect.
       * @return A connection to the SortSignal.
       */
      boost::signals2::connection connect_sort_signal(
        const SortSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable SortSignal m_sort_signal;
      std::shared_ptr<ListModel<TableHeaderItem::Model>> m_items;
      std::shared_ptr<TableFilter> m_filter;
      std::shared_ptr<ListModel<int>> m_widths;
      std::vector<TableHeaderItem*> m_item_views;
      TranslatedListModel<int> m_translation;
      int m_resize_index;
      ScrollBox* m_scroll_box;
      QBoxLayout* m_body_layout;
      PopupBox* m_filter_panel;
      QWidget* m_closing_filter_panel;
      QPointer<QVariantAnimation> m_filter_animation;
      bool m_is_closing_filters;
      boost::signals2::scoped_connection m_items_connection;
      boost::signals2::scoped_connection m_filter_connection;
      boost::signals2::scoped_connection m_widths_connection;

      void on_items_operation(
        const ListModel<TableHeaderItem::Model>::Operation& operation);
      void on_start_resize(int index);
      void on_end_resize(int index);
      void close_filter_panel();
      void open_filter_panel(int index);
      void swap_filter_panel(int index);
      void on_filter(int column, TableFilter::Filter filter);
      void on_widths_operation(const ListModel<int>::Operation& operation);
      void on_sort(int index, TableHeaderItem::Order order);
      void on_filter_open(int index, bool is_open);
  };
}

#endif
