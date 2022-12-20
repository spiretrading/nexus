#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTask.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the key bindings window. */
  class KeyBindingsWindow : public Window {
    public:

      /**
       * Signals the submission of key bindings.
       * @param submission The key bindings of the order tasks to be submitted.
       */
      using SubmitSignal = Signal<void (
        const std::shared_ptr<ListModel<OrderTask>>& submission)>;

      /**
       * Constructs a KeyBindingsWindow.
       * @param key_bindings The key bindings to represent.
       * @param parent The parent widget.
       */
      explicit KeyBindingsWindow(std::shared_ptr<KeyBindingsModel> key_bindings,
        QWidget* parent = nullptr);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      NavigationView* m_navigation_view;
      OrderTasksPage* m_order_tasks_page;
      std::shared_ptr<ListModel<OrderTask>> m_order_tasks_submission;
      bool m_is_modified;
      boost::signals2::scoped_connection m_order_tasks_connection;

      void on_apply();
      void on_cancel();
      void on_ok();
      void on_reset();
      void on_order_task_operation(
        const ListModel<OrderTask>::Operation& operation);
  };
}

#endif
