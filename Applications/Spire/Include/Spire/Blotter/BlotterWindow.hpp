#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include <memory>
#include <boost/signals2/connection.hpp>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterOrderLogView.hpp"
#include "Spire/Blotter/BlotterTaskView.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays the blotter window. */
  class BlotterWindow : public Window {
    public:

      /**
       * Constructs a BlotterWindow.
       * @param blotter The blotter to represent.
       * @param parent The parent widget.
       */
      explicit BlotterWindow(
        std::shared_ptr<BlotterModel> blotter, QWidget* parent = nullptr);

      /** Returns the window's BlotterTaskView. */
      BlotterTaskView& get_task_view();

      /** Returns the window's BlotterOrderLogView. */
      BlotterOrderLogView& get_order_log_view();

    private:
      std::shared_ptr<BlotterModel> m_blotter;
      BlotterTaskView* m_task_view;
      BlotterOrderLogView* m_order_log_view;
      boost::signals2::scoped_connection m_name_connection;

      void on_name_update(const QString& name);
  };
}

#endif
