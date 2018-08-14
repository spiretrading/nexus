#include "spire/book_view_ui_tester/book_view_test_controller_window.hpp"
#include <QGridLayout>
#include "spire/book_view/book_view_window.hpp"
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
  auto layout = new QGridLayout(this);
  auto loading_time_label = new QLabel("Loading Time (ms):", this);
  layout->addWidget(loading_time_label, 0, 0);
  m_load_time_spin_box = new QSpinBox(this);
  m_load_time_spin_box->setMaximum(100000);
  m_load_time_spin_box->setValue(4000);
  layout->addWidget(m_load_time_spin_box, 0, 1);
  auto m_model_update_period_label = new QLabel("Model Update Period (ms):",
    this);
  layout->addWidget(m_model_update_period_label, 1, 0);
  m_model_update_period_spin_box = new QSpinBox(this);
  m_model_update_period_spin_box->setMaximum(100000);
  m_model_update_period_spin_box->setValue(1000);
  connect(m_model_update_period_spin_box, &QSpinBox::editingFinished,
    this, &BookViewTestControllerWindow::on_model_period_updated);
  layout->addWidget(m_model_update_period_spin_box, 1, 1);
}

void BookViewTestControllerWindow::on_security_changed(
    const Security& security) {
  m_model = std::make_shared<RandomBookViewModel>(security,
    boost::posix_time::millisec(m_load_time_spin_box->value()),
    *m_timer_thread_pool);
  m_model->set_period(boost::posix_time::milliseconds(
      m_model_update_period_spin_box->value()));
  m_window->set_model(m_model);
}

void BookViewTestControllerWindow::on_model_period_updated() {
  if(m_model != nullptr) {
    m_model->set_period(boost::posix_time::milliseconds(
      m_model_update_period_spin_box->value()));
  }
}
