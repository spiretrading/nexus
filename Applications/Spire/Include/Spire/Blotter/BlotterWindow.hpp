#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays a blotter. */
  class BlotterWindow : public Window {
    public:

      /**
       * Signals a request to cancel the selected tasks.
       * @param tasks The tasks selected when cancellation was requested.
       */
      using CancelTasksSignal = BlotterTasksView::CancelTasksSignal;

      /**
       * Constructs a BlotterWindow.
       * @param model The blotter to display.
       * @param parent The parent widget.
       */
      BlotterWindow(
        std::shared_ptr<BlotterModel> model, QWidget* parent = nullptr);

      /** Returns the displayed blotter's model. */
      const std::shared_ptr<BlotterModel>& get_model() const;

      /** Connects a slot to the CancelTasksSignal. */
      boost::signals2::connection connect_cancel_tasks_signal(
        const CancelTasksSignal::slot_type& slot) const;

    private:
      std::shared_ptr<BlotterModel> m_model;
      BlotterTasksView* m_tasks_view;
      boost::signals2::scoped_connection m_name_connection;

      void on_name(const QString& name);
  };
}

#endif
