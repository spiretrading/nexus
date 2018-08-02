#include "spire/book_view_ui_tester/book_view_test_controller_window.hpp"
#include "spire/book_view/book_view_window.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Spire;

BookViewTestControllerWindow::BookViewTestControllerWindow(
    BookViewWindow* window) {
  setFixedSize(scale(400, 200));
}
