#ifndef SPIRE_BOOK_VIEW_TEST_CONTROLLER_WINDOW
#define SPIRE_BOOK_VIEW_TEST_CONTROLLER_WINDOW
#include <QWidget>
#include "spire/book_view/book_view.hpp"

namespace Spire {

  //! Displays the window used to control how a book view window is tested.
  class BookViewTestControllerWindow : public QWidget {
    public:
      BookViewTestControllerWindow(BookViewWindow* window);
  };
}

#endif
