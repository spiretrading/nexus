#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include <boost/uuid/uuid.hpp>
#include "Spire/Blotter/BlotterCollection.hpp"
#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Blotter/OrderLogView.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {
  class BlotterTabView;
  class SplitView;

  /** Displays a blotter. */
  class BlotterWindow : public Window, public LegacyUI::PersistentWindow {
    public:
      using RunTasksSignal = BlotterTasksView::RunSignal;
      using CancelTasksSignal = BlotterTasksView::CancelSignal;
      using CancelTaskOperationSignal = BlotterTasksView::CancelOperationSignal;
      using CancelOrdersSignal = OrderLogView::CancelSignal;
      using CancelOrderOperationSignal = OrderLogView::CancelOperationSignal;
      using FlattenPositionsSignal = BlotterPositionsView::FlattenSignal;

      /**
       * Constructs a BlotterWindow.
       * @param model The blotter to display.
       * @param collection The blotters available for linking.
       * @param cancel_bindings The user's cancel key bindings.
       * @param position_bindings The user's position key bindings.
       * @param parent The parent widget.
       */
      BlotterWindow(std::shared_ptr<BlotterModel> model,
        std::shared_ptr<BlotterCollection> collection,
        std::shared_ptr<CancelKeyBindingsModel> cancel_bindings,
        std::shared_ptr<PositionKeyBindingsModel> position_bindings,
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

      /** Connects a slot to the FlattenPositionsSignal. */
      boost::signals2::connection connect_flatten_positions_signal(
        const FlattenPositionsSignal::slot_type& slot) const;

      std::unique_ptr<LegacyUI::WindowSettings> GetWindowSettings() const
        override;

    private:
      friend class BlotterController;
      friend class BlotterWindowSettings;
      boost::uuids::uuid m_id;
      std::shared_ptr<BlotterModel> m_model;
      std::shared_ptr<BlotterCollection> m_collection;
      BlotterTasksView* m_tasks_view;
      BlotterTabView* m_tab_view;
      SplitView* m_split_view;
      boost::signals2::scoped_connection m_name_connection;

      void on_name(const QString& name);
      void on_context_menu(const QPoint& position);
      void on_rename();
  };
}

#endif
