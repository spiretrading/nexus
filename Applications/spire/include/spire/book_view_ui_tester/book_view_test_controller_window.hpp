#ifndef SPIRE_BOOK_VIEW_TEST_CONTROLLER_WINDOW
#define SPIRE_BOOK_VIEW_TEST_CONTROLLER_WINDOW
#include <QWidget>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Nexus/Definitions/Security.hpp>
#include "spire/book_view/book_view.hpp"

namespace Spire {

  //! Displays the window used to control how a book view window is tested.
  class BookViewTestControllerWindow : public QWidget {
    public:
      BookViewTestControllerWindow(BookViewWindow* window,
        Beam::Threading::TimerThreadPool& thread_timer_pool);

    private:
      BookViewWindow* m_window;
      Beam::Threading::TimerThreadPool* m_timer_thread_pool;

      void on_security_changed(const Nexus::Security& security);
  };
}

#endif
