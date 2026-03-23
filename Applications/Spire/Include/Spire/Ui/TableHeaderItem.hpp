#ifndef SPIRE_TABLE_HEADER_ITEM_HPP
#define SPIRE_TABLE_HEADER_ITEM_HPP
#include <boost/signals2/connection.hpp>
#include <QHBoxLayout>
#include <QPointer>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/ClickObserver.hpp"
#include "Spire/Ui/TableFilter.hpp"

namespace Spire {
  class ResponsiveLabel;

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
      using FilterButton = Styles::StateSelector<void, struct FilterButtonTag>;

      /** Identifies styling for the active element sub-component. */
      using ActiveElement =
        Styles::StateSelector<void, struct ActiveElementTag>;

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

      /**
       * Signals an action to change this column's sort order.
       * @param order The sort order to update this column to.
       */
      using SortSignal = Signal<void (Order order)>;

      /**
       * Constructs a TableHeaderItem.
       * @param model This item's model.
       * @param parent The parent widget.
       */
      explicit TableHeaderItem(
        std::shared_ptr<ValueModel<Model>> model, QWidget* parent = nullptr);

      /** Returns this item's model. */
      const std::shared_ptr<ValueModel<Model>>& get_model() const;

      /**
       * Returns the boolean model used to track whether the filter panel is
       * currently open.
       */
      const std::shared_ptr<BooleanModel>& is_filter_open() const;

      /** Returns <code>true</code> iff this item is resizeable. */
      bool is_resizeable() const;

      /** Sets whether this item can be resized. */
      void set_is_resizeable(bool is_resizeable);

      /** Connects a slot to the SortSignal. */
      boost::signals2::connection connect_sort_signal(
        const SortSignal::slot_type& slot) const;

      QSize minimumSizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable SortSignal m_sort_signal;
      std::shared_ptr<ValueModel<Model>> m_model;
      std::shared_ptr<BooleanModel> m_is_filter_open;
      ClickObserver m_click_observer;
      bool m_is_resizeable;
      ResponsiveLabel* m_name_label;
      QPointer<QWidget> m_filter_control;
      QWidget* m_active_element;
      QWidget* m_active_indicator;
      QWidget* m_active_container;
      QWidget* m_sort_indicator;
      QPointer<QWidget> m_sash;
      QHBoxLayout* m_controls_layout;
      bool m_is_resizing;
      QFont m_font;
      boost::signals2::scoped_connection m_connection;
      boost::signals2::scoped_connection m_label_name_connection;
      boost::signals2::scoped_connection m_label_style_connection;

      void on_click();
      void on_filter_checked(bool checked);
      void on_update(const Model& model);
      void on_label_name_update(const QString& name);
      void on_label_style();
  };
}

#endif
