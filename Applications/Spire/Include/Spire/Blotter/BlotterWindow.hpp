#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include "Spire/Blotter/BlotterTabView.hpp"
#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays a blotter. */
  class BlotterWindow : public Window {
    public:
      using RunTasksSignal = BlotterTasksView::RunSignal;
      using CancelTasksSignal = BlotterTasksView::CancelSignal;
      using CancelTaskOperationSignal = BlotterTasksView::CancelOperationSignal;
      using CancelOrdersSignal = OrderLogView::CancelSignal;
      using CancelOrderOperationSignal = OrderLogView::CancelOperationSignal;

      /**
       * Constructs a BlotterWindow.
       * @param model The blotter to display.
       * @param bindings The user's cancel key bindings.
       * @param parent The parent widget.
       */
      BlotterWindow(std::shared_ptr<BlotterModel> model,
        std::shared_ptr<CancelKeyBindingsModel> bindings,
        QWidget* parent = nullptr);

      /** Returns the displayed blotter's model. */
      const std::shared_ptr<BlotterModel>& get_model() const;

      /** Connects a slot to the RunTasksSignal. */
      boost::signals2::connection connect_run_tasks_signal(
        const RunTasksSignal::slot_type& slot) const;

      /** Connects a slot to the CancelTasksSignal. */
      boost::signals2::connection connect_cancel_tasks_signal(
        const CancelTasksSignal::slot_type& slot) const;

      /** Connects a slot to the CancelTaskOperationSignal. */
      boost::signals2::connection connect_cancel_task_operation_signal(
        const CancelTaskOperationSignal::slot_type& slot) const;

      /** Connects a slot to the CancelOrdersSignal. */
      boost::signals2::connection connect_cancel_orders_signal(
        const CancelOrdersSignal::slot_type& slot) const;

      /** Connects a slot to the CancelOrderOperationSignal. */
      boost::signals2::connection connect_cancel_order_operation_signal(
        const CancelOrderOperationSignal::slot_type& slot) const;

    private:
      std::shared_ptr<BlotterModel> m_model;
      BlotterTasksView* m_tasks_view;
      BlotterTabView* m_tab_view;
      boost::signals2::scoped_connection m_name_connection;

      void on_name(const QString& name);
  };
}

#endif
