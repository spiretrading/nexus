#ifndef SPIRE_LOGIN_WINDOW_HPP
#define SPIRE_LOGIN_WINDOW_HPP
#include <string>
#include <boost/signals2/connection.hpp>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPoint>
#include <QPushButton>
#include <QtSvg/QSvgWidget>
#include <QWidget>
#include "spire/login/login.hpp"
#include "spire/login/chroma_hash_widget.hpp"
#include "spire/ui/flat_button.hpp"
#include "spire/ui/icon_button.hpp"

namespace spire {

  //! \brief Displays the login window.
  class login_window : public QWidget {
    public:

      //! The login state to display to the user.
      enum class state {

        //! Nothing to report.
        NONE,

        //! The user is in the process of logging in.
        LOGGING_IN,

        //! The user requested to cancel.
        CANCELLING,

        //! The login attempt used incorrect credentials.
        INCORRECT_CREDENTIALS,

        //! The server is unavailable.
        SERVER_UNAVAILABLE
      };

      //! Signals an attempt to login.
      /*!
        \param username The username to login with.
        \param password The password to login with.
      */
      using login_signal = signal<void (const std::string& username,
        const std::string& password)>;

      //! Signals to cancel a previous login operation.
      using cancel_signal = signal<void ()>;

      //! Constructs a login window in the NONE state.
      login_window(const std::string& version, QWidget* parent = nullptr);

      //! Sets the state to display to the user.
      void set_state(state state);

      //! Connects a slot to the login signal.
      boost::signals2::connection connect_login_signal(
        const login_signal::slot_type& slot) const;

      //! Connects a slot to the cancel signal.
      boost::signals2::connection connect_cancel_signal(
        const cancel_signal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;

      void keyPressEvent(QKeyEvent* event);

      void mouseMoveEvent(QMouseEvent* event) override;

      void mousePressEvent(QMouseEvent* event) override;

      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable login_signal m_login_signal;
      mutable cancel_signal m_cancel_signal;
      state m_state;
      QSvgWidget* m_logo_widget;
      QLabel* m_status_label;
      QLineEdit* m_username_lineedit;
      QLineEdit* m_password_lineedit;
      spire::flat_button* m_sign_in_button;
      spire::icon_button* m_exit_button;
      bool m_is_dragging;
      QPoint m_last_pos;
      spire::chroma_hash_widget* m_chroma_hash_widget;

      void reset_widget();
      void reset_visuals();
      void on_button_click();
      void inputs_updated();
      void password_input_changed();
      void enable_button();
      void disable_button();
      void button_focused();
  };
}

#endif
