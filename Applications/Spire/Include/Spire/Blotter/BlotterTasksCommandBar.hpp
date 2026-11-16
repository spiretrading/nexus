#ifndef SPIRE_BLOTTER_TASKS_COMMAND_BAR_HPP
#define SPIRE_BLOTTER_TASKS_COMMAND_BAR_HPP
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {
  class Button;

  /** Displays commands for a blotter and its order execution tasks. */
  class BlotterTasksCommandBar : public QWidget {
    public:

      /** Signals a request to run the selected tasks. */
      using RunSignal = Signal<void ()>;

      /** Signals a request to cancel the selected tasks. */
      using CancelSignal = Signal<void ()>;

      /**
       * Constructs a BlotterTasksCommandBar.
       * @param model The blotter to control.
       * @param parent The parent widget.
       */
      explicit BlotterTasksCommandBar(
        std::shared_ptr<BlotterModel> model, QWidget* parent = nullptr);

      /** Connects a slot to the RunSignal. */
      boost::signals2::connection connect_run_signal(
        const RunSignal::slot_type& slot) const;

      /** Connects a slot to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    private:
      mutable RunSignal m_run_signal;
      mutable CancelSignal m_cancel_signal;
      std::shared_ptr<BlotterModel> m_model;
      Button* m_active;
      boost::signals2::scoped_connection m_active_connection;

      void on_active();
      void on_active_update(bool is_active);
  };
}

#endif
