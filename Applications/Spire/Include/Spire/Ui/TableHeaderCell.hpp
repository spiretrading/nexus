#ifndef SPIRE_TABLE_HEADER_CELL_HPP
#define SPIRE_TABLE_HEADER_CELL_HPP
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/Spire/CompositeValueModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/StateSelector.hpp"

namespace Spire {

  /** Displays a single cell within a TableViewHeader. */
  class TableHeaderCell : public QWidget {
    public:

      /** Styles this column when it's sort order is not UNORDERED. */
      using Sortable = Styles::StateSelector<void, struct SortableTag>;

      /** Identifies styling for the label sub-component. */
      using Label = Styles::StateSelector<void, struct LabelTag>;

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

      /** Stores this cell's model. */
      struct Model {

        /** The name of the column. */
        QString m_name;

        /** The short form name of the column. */
        QString m_short_name;

        /** The column's sort order. */
        Order m_order;

        /** Whether the column can be hidden. */
        bool m_is_hideable;

        /** Whether the column is currently visible. */
        bool m_is_visible;
      };

      /** Signals an action to hide this column. */
      using HideSignal = Signal<void ()>;

      /**
       * Signals an action to change this column's sort order.
       * @param order The sort order to update this column to.
       */
      using SortSignal = Signal<void (Order order)>;

      /**
       * Constructs a TableHeaderCell.
       * @param model This cell's model.
       * @param parent The parent widget.
       */
      explicit TableHeaderCell(
        std::shared_ptr<CompositeValueModel<Model>> model,
        QWidget* parent = nullptr);

      /** Returns this cell's model. */
      const std::shared_ptr<CompositeValueModel<Model>>& get_model() const;

      /** Connects a slot to the HideSignal. */
      boost::signals2::connection connect_hide_signal(
        const HideSignal::slot_type& slot) const;

      /** Connects a slot to the SortSignal. */
      boost::signals2::connection connect_sort_signal(
        const SortSignal::slot_type& slot) const;

    private:
      mutable HideSignal m_hide_signal;
      mutable SortSignal m_sort_signal;
      std::shared_ptr<CompositeValueModel<Model>> m_model;
      boost::signals2::scoped_connection m_order_connection;

      void on_order(Order order);
  };
}

#endif
