#include "spire/book_view_ui_tester/book_view_test_controller_window.hpp"
#include "spire/book_view/book_view_window.hpp"
#include "spire/book_view_ui_tester/random_book_view_model.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Nexus;
using namespace Spire;

BookViewTestControllerWindow::BookViewTestControllerWindow(
    BookViewWindow* window,
    Beam::Threading::TimerThreadPool& timer_thread_pool)
    : m_window(window),
      m_timer_thread_pool(&timer_thread_pool) {
  setFixedSize(scale(400, 200));
  m_window->connect_security_change_signal(
    [=] (const auto& s) { on_security_changed(s); });
}

void BookViewTestControllerWindow::on_security_changed(
    const Security& security) {
  auto model = std::make_shared<RandomBookViewModel>(security,
    boost::posix_time::millisec(1000), *m_timer_thread_pool);
  m_window->set_model(model);
}
