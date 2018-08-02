#include "spire/login_ui_tester/login_ui_tester.hpp"
#include <QVBoxLayout>
#include "spire/login/login_window.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

login_ui_tester::login_ui_tester(spire::LoginWindow* login, QWidget* parent)
    : QWidget(parent),
      m_login_window(login) {
  setFixedSize(scale(480, 270));
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

bool login_ui_tester::eventFilter(QObject* receiver, QEvent* event) {
  if(event->type() == QEvent::MouseButtonRelease && event->type() != QEvent::Move) {
    if(receiver == m_accept_button) {
      m_login_window->set_state(LoginWindow::State::NONE);
    } else if(receiver == m_reject_button) {
      m_login_window->set_state(LoginWindow::State::INCORRECT_CREDENTIALS);
    } else if(receiver == m_server_unavailable_button) {
      m_login_window->set_state(LoginWindow::State::SERVER_UNAVAILABLE);
    }
  }
  if(receiver == m_login_window && event->type() == QEvent::Close) {
    close();
  }
  return QWidget::eventFilter(receiver, event);
}
