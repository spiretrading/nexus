#ifndef SPIRE_BLOTTER_WINDOW_HPP
#define SPIRE_BLOTTER_WINDOW_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays the blotter window. */
  class BlotterWindow : public Window {
    public:

      /** Constructs a blotter window. */
      explicit BlotterWindow(QWidget* parent = nullptr);

    private:
      QWidget* m_body;
      BlotterTaskView* m_task_view;
      TabView* m_tab_view;
      SplitView* m_split_view;
      BlotterStatusBar* m_status_bar;
  };
}

#endif
