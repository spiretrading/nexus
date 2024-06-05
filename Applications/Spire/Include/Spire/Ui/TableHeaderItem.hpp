#ifndef SPIRE_TABLE_HEADER_ITEM_HPP
#define SPIRE_TABLE_HEADER_ITEM_HPP
#include <boost/signals2/connection.hpp>
#include <QBoxLayout>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/TableFilter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a single item within a TableViewHeader. */
  class TableHeaderItem : public QWidget {
    public:

      /** Styles this column when it's sort order is not UNORDERED. */
      using Sortable = Styles::StateSelector<void, struct SortableTag>;

      /** Styles this column when a filter is applied to it. */
      using Filtered = Styles::StateSelector<void, struct FilteredTag>;

      /** Identifies styling for the label sub-component. */
      using Label = Styles::StateSelector<void, struct LabelTag>;

      /** Identifies styling for the filter button sub-component. */
      using FilterButton = Styles::StateSelector<void, struct FilterButton>;

      /** Identifies styling for the hover element sub-component. */
      using HoverElement = Styles::StateSelector<void, struct HoverElementTag>;

      /** Specifies whether and how the column is sorted. */
      enum class Order {

        /** The column is unsorted. */
        NONE,

        /** The column has no sort order, it can not be sorted. */
        UNORDERED,

        /** The column is sorted from smallest to biggest. */
        ASCENDING,

        /** The column is sorted from biggest to smallest. */
        DESCENDING
      };

      /** Stores this item's model. */
      struct Model {

        /** The name of the column. */
        QString m_name;

        /** The short form name of the column. */
        QString m_short_name;

        /** The column's sort order. */
        Order m_order;

        /** How this column is filtered. */
        TableFilter::Filter m_filter;
      };

      /** Signals an action to start a column resize. */
      using StartResizeSignal = Signal<void ()>;

      /** Signals an action to end a column resize. */
      using EndResizeSignal = Signal<void ()>;

      /**
       * Signals an action to change this column's sort order.
       * @param order The sort order to update this column to.
       */
      using SortSignal = Signal<void (Order order)>;

      /** Signals an action to filter this column. */
      using FilterSignal = Signal<void ()>;

      /**
       * Constructs a TableHeaderItem.
       * @param model This item's model.
       * @param parent The parent widget.
       */
      explicit TableHeaderItem(
        std::shared_ptr<ValueModel<Model>> model, QWidget* parent = nullptr);

      /** Returns this item's model. */
      const std::shared_ptr<ValueModel<Model>>& get_model() const;

      /** Returns the filter button. */
      Button& get_filter_button();

      /** Returns <code>true</code> iff this item is resizeable. */
      bool is_resizeable() const;

      /** Sets whether this item can be resized. */
      void set_is_resizeable(bool is_resizeable);

      /** Connects a slot to the start resize signal. */
      boost::signals2::connection connect_start_resize_signal(
        const StartResizeSignal::slot_type& slot) const;

      /** Connects a slot to the end resize signal. */
      boost::signals2::connection connect_end_resize_signal(
        const EndResizeSignal::slot_type& slot) const;

      /** Connects a slot to the SortSignal. */
      boost::signals2::connection connect_sort_signal(
        const SortSignal::slot_type& slot) const;

      /** Connects a slot to the FilterSignal. */
      boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const;

      QSize minimumSizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable StartResizeSignal m_start_resize_signal;
      mutable EndResizeSignal m_end_resize_signal;
      mutable SortSignal m_sort_signal;
      std::shared_ptr<ValueModel<Model>> m_model;
      bool m_is_resizeable;
      Button* m_filter_button;
      QWidget* m_sort_indicator;
      QWidget* m_sash;
      QBoxLayout* m_contents_layout;
      boost::signals2::scoped_connection m_connection;

      void on_update(const Model& model);
  };
}

#endif
