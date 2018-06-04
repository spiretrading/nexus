#ifndef SPIRE_LOGIN_UI_TESTER_HPP
#define SPIRE_LOGIN_UI_TESTER_HPP
#include <QEvent>
#include <QPushButton>
#include <QWidget>
#include "spire/login/login.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays a window to simulate account login, rejection, and server
  //  unavailable status'.
  class login_ui_tester : public QWidget {
    public:

    //! Constructs the login_ui_tester.
    /*!
      \param window The login_window that this widget will interact with.
      \param frame The top-
    */
    login_ui_tester(spire::login_window* login, QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* receiver, QEvent* event);

    private:
      QPushButton* m_accept_button;
      QPushButton* m_reject_button;
      QPushButton* m_server_unavailable_button;
      login_window* m_login_window;
  };
}

#endif
