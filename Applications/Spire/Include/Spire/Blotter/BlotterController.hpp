#ifndef SPIRE_BLOTTER_CONTROLLER_HPP
#define SPIRE_BLOTTER_CONTROLLER_HPP
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"

namespace Spire {

  /** Controls a blotter's tasks and window. */
  class BlotterController {
    public:

      /**
       * Constructs a BlotterController.
       * @param model The controlled blotter.
       * @param tasks The blotter's local task source.
       * @param collection The blotters available for linking.
       * @param cancel_bindings The user's cancel key bindings.
       * @param position_bindings The user's position key bindings.
       * @param profile Supplies task execution services.
       */
      BlotterController(std::shared_ptr<BlotterModel> model,
        std::shared_ptr<TasksModel> tasks,
        std::shared_ptr<BlotterCollection> collection,
        std::shared_ptr<CancelKeyBindingsModel> cancel_bindings,
        std::shared_ptr<PositionKeyBindingsModel> position_bindings,
        Beam::Ref<UserProfile> profile);

      ~BlotterController();

      /** Returns the controlled blotter. */
      const std::shared_ptr<BlotterModel>& get_model() const;

      /** Returns whether the window is open. */
      const std::shared_ptr<const BooleanModel>& is_open() const;

      /**
       * Adds a task without executing it.
       * @param node The task to add.
       */
      std::shared_ptr<Task> add(const CanvasNode& node);

      /**
       * Adds and executes a task.
       * @param node The task to execute.
       */
      std::shared_ptr<Task> submit(const CanvasNode& node);

      /**
       * Executes a cancel operation for a ticker.
       * @param operation The cancellation rule.
       * @param ticker The ticker to match.
       * @param prices Displayed order prices used for ranking.
       * @param ids Eligible order IDs, or none for all matching orders.
       */
      void cancel(CancelKeyBindingsModel::Operation operation,
        const Nexus::Ticker& ticker,
        const std::unordered_map<Nexus::OrderId, Nexus::Money>& prices,
        const boost::optional<std::vector<Nexus::OrderId>>& ids);

      /** Opens or raises the window. */
      BlotterWindow& open();

      /** Closes the window. */
      void close();

    private:
      class EventFilter;
      std::shared_ptr<BlotterModel> m_model;
      std::shared_ptr<TasksModel> m_tasks;
      std::shared_ptr<BlotterCollection> m_collection;
      std::shared_ptr<CancelKeyBindingsModel> m_cancel_bindings;
      std::shared_ptr<PositionKeyBindingsModel> m_position_bindings;
      UserProfile* m_profile;
      std::shared_ptr<LocalBooleanModel> m_is_open;
      std::shared_ptr<const BooleanModel> m_read_only_is_open;
      std::unique_ptr<BlotterWindow> m_window;
      std::unique_ptr<EventFilter> m_event_filter;
      QtTaskQueue m_events;

      BlotterController(const BlotterController&) = delete;
      BlotterController& operator =(const BlotterController&) = delete;
      void on_state(
        const std::weak_ptr<Task>& task, const Task::StateEntry& state);
      void on_close();
  };
}

#endif
