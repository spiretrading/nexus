#ifndef SPIRE_BLOTTER_TAB_VIEW_HPP
#define SPIRE_BLOTTER_TAB_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/BlotterTaskEntry.hpp"
#include "Spire/Blotter/OrderLogView.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Displays a blotter's secondary views. */
  class BlotterTabView : public QWidget {
    public:

      /**
       * Constructs a BlotterTabView.
       * @param tasks The blotter's tasks.
       * @param current The current task index in source table coordinates.
       * @param bindings The user's cancel key bindings.
       * @param parent The parent widget.
       */
      BlotterTabView(std::shared_ptr<TasksModel> tasks,
        std::shared_ptr<TableView::CurrentModel> current,
        std::shared_ptr<CancelKeyBindingsModel> bindings,
        QWidget* parent = nullptr);

      /** Connects a slot to the order log's CancelSignal. */
      boost::signals2::connection connect_cancel_orders_signal(
        const OrderLogView::CancelSignal::slot_type& slot) const;

      /** Connects a slot to the order log's CancelOperationSignal. */
      boost::signals2::connection connect_cancel_order_operation_signal(
        const OrderLogView::CancelOperationSignal::slot_type& slot) const;

    private:
      OrderLogView* m_order_log;
  };
}

#endif
