#ifndef SPIRE_LOGIN_WINDOW_HPP
#define SPIRE_LOGIN_WINDOW_HPP
#include <string>
#include <QLabel>
#include <QPoint>
#include <QWidget>
#include "Spire/Login/Login.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the login window. */
  class LoginWindow : public QWidget {
    public:

      /** The login state to display to the user. */
      enum class State {

        /** Nothing to report. */
        NONE,

        /** The user is in the process of logging in. */
        LOGGING_IN,

        /** The user requested to cancel. */
        CANCELLING,

        /** The login attempt used incorrect credentials. */
        INCORRECT_CREDENTIALS,

        /** The server is unavailable. */
        SERVER_UNAVAILABLE
      };

      /**
       * Signals an attempt to login.
       * @param username The username to login with.
       * @param password The password to login with.
       */
      using LoginSignal =
        Signal<void (const std::string& username, const std::string& password)>;

      /** Signals to cancel a previous login operation. */
      using CancelSignal = Signal<void ()>;

      /**
       * Constructs a login window in the NONE state.
       * @param version The application build version.
       */
      explicit LoginWindow(
        const std::string& version, QWidget* parent = nullptr);

      /** Sets the state to display to the user. */
      void set_state(State state);

      /** Connects a slot to the login signal. */
      boost::signals2::connection connect_login_signal(
        const LoginSignal::slot_type& slot) const;

      /** Connects a slot to the cancel signal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable LoginSignal m_login_signal;
      mutable CancelSignal m_cancel_signal;
      State m_state;
      DropShadow* m_shadow;
      QLabel* m_logo_widget;
      TextBox* m_status_label;
      TextBox* m_username_text_box;
      TextBox* m_password_text_box;
      Button* m_sign_in_button;
      bool m_is_dragging;
      QPoint m_last_pos;
      ChromaHashWidget* m_chroma_hash_widget;

      void reset_all();
      void reset_visuals();
      void try_login();
  };
}

#endif
