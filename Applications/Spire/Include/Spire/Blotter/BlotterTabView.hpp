#ifndef SPIRE_BLOTTER_TAB_VIEW_HPP
#define SPIRE_BLOTTER_TAB_VIEW_HPP
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Blotter/OrderLogView.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
  class BlotterExecutionsView;
  class TabView;

  /** Displays a blotter's secondary views. */
  class BlotterTabView : public QWidget {
    public:

      /**
       * Constructs a BlotterTabView.
       * @param model The blotter to display.
       * @param current The current task index in source table coordinates.
       * @param cancel_bindings The user's cancel key bindings.
       * @param position_bindings The user's position key bindings.
       * @param parent The parent widget.
       */
      BlotterTabView(std::shared_ptr<BlotterModel> model,
        std::shared_ptr<TableView::CurrentModel> current,
        std::shared_ptr<CancelKeyBindingsModel> cancel_bindings,
        std::shared_ptr<PositionKeyBindingsModel> position_bindings,
        QWidget* parent = nullptr);

      /** Connects a slot to the order log's CancelSignal. */
      boost::signals2::connection connect_cancel_orders_signal(
        const OrderLogView::CancelSignal::slot_type& slot) const;

      /** Connects a slot to the order log's CancelOperationSignal. */
      boost::signals2::connection connect_cancel_order_operation_signal(
        const OrderLogView::CancelOperationSignal::slot_type& slot) const;

      /** Connects a slot to the positions view's FlattenSignal. */
      boost::signals2::connection connect_flatten_positions_signal(
        const BlotterPositionsView::FlattenSignal::slot_type& slot) const;

    private:
      friend class BlotterWindowSettings;
      TabView* m_tab_view;
      BlotterPositionsView* m_positions;
      OrderLogView* m_order_log;
      BlotterExecutionsView* m_executions;
  };
}

#endif
