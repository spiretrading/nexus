#include "spire/login_ui_tester/login_ui_tester.hpp"
#include <QVBoxLayout>

using namespace boost;
using namespace boost::signals2;
using namespace spire;

login_ui_tester::login_ui_tester(login_window* window, QWidget* parent)
    : QWidget(parent) {
  m_login_window = window;
  setFixedSize(480, 270);
  auto layout = new QVBoxLayout(this);
  m_accept_button = new QPushButton("Accept Account", this);
  m_accept_button->installEventFilter(this);
  m_accept_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_accept_button->setStyleSheet("background-color: green;");
  layout->addWidget(m_accept_button);
  m_reject_button = new QPushButton("Reject Account", this);
  m_reject_button->installEventFilter(this);
  m_reject_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_reject_button->setStyleSheet("background-color: red;");
  layout->addWidget(m_reject_button);
  m_server_unavailable_button = new QPushButton("Server Unavailable", this);
  m_server_unavailable_button->installEventFilter(this);
  m_server_unavailable_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_server_unavailable_button->setStyleSheet("background-color: yellow;");
  layout->addWidget(m_server_unavailable_button);
}

bool login_ui_tester::eventFilter(QObject* obj, QEvent* event) {
  if(event->type() == QEvent::MouseButtonRelease && event->type() != QEvent::Move) {
    if(obj == m_accept_button) {
      m_login_window->set_state(login_window::state::NONE);
    } else if(obj == m_reject_button) {
      m_login_window->set_state(login_window::state::INCORRECT_CREDENTIALS);
    } else if(obj == m_server_unavailable_button) {
      m_login_window->set_state(login_window::state::SERVER_UNAVAILABLE);
    }
  }
  if(obj == m_login_window && event->type() == QEvent::Close) {
    close();
  }
  return QWidget::eventFilter(obj, event);
}
