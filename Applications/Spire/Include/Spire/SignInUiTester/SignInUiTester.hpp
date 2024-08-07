#ifndef SPIRE_SIGN_IN_UI_TESTER_HPP
#define SPIRE_SIGN_IN_UI_TESTER_HPP
#include <QEvent>
#include <QPushButton>
#include <QWidget>
#include "Spire/SignIn/SignIn.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a window to simulate account sign in, rejection, and server
   * unavailable status.
   */
  class SignInUiTester : public QWidget {
    public:

      /**
       * Constructs the SignInUiTester.
       * @param window The SignInWindow that this widget will interact with.
       * @param frame The top-
       */
      explicit SignInUiTester(SignInWindow& window, QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* receiver, QEvent* event);

    private:
      QPushButton* m_accept_button;
      QPushButton* m_reject_button;
      QPushButton* m_server_unavailable_button;
      SignInWindow* m_window;
  };
}

#endif
