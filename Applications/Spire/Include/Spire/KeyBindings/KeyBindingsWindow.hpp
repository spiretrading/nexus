#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTask.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the key bindings window. */
  class KeyBindingsWindow : public Window {
    public:

      /**
       * Signals the submission of key bindings.
       * @param order_tasks_submission The key bindings of the order tasks
       *        to be submitted.
       */
      using SubmitSignal = Signal<void (
        const std::shared_ptr<ListModel<OrderTask>>& order_tasks_submission)>;

      /** Stores the various models used to build a KeyBindingsWindow. */
      struct KeyBindingsModels {

        /** The model used to query region matches. */
        std::shared_ptr<ComboBox::QueryModel> m_region_query_model;

        /** The order tasks model. */
        std::shared_ptr<ListModel<OrderTask>> m_order_tasks_model;

        /** The default order tasks model. */
        std::shared_ptr<ListModel<OrderTask>> m_default_order_tasks_model;
      };

      /**
       * Constructs a KeyBindingsWindow.
       * @param models The models used by the KeyBindingsWindow.
       * @param destination_database The destination database.
       * @param market_database The market database.
       * @param parent The parent widget.
       */
      KeyBindingsWindow(KeyBindingsModels models, 
        const Nexus::DestinationDatabase& destination_database,
        const Nexus::MarketDatabase& market_database,
        QWidget* parent = nullptr);

      /** Returns the model used to query region matches. */
      const std::shared_ptr<ComboBox::QueryModel>&
        get_region_query_model() const;

      /** Returns the list model of the order task key bindings. */
      const std::shared_ptr<ListModel<OrderTask>>& get_order_task_model() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      mutable SubmitSignal m_submit_signal;
      NavigationView* m_navigation_view;
      OrderTasksPage* m_order_tasks_page;
      std::shared_ptr<ListModel<OrderTask>> m_order_tasks_submission;
      std::shared_ptr<ListModel<OrderTask>> m_default_order_tasks_model;

      void on_apply();
      void on_cancel();
      void on_ok();
      void on_reset();
  };
}

#endif
