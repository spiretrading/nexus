#ifndef SPIRE_LOGIN_WINDOW_HPP
#define SPIRE_LOGIN_WINDOW_HPP
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "spire/login/login.hpp"

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
      login_window(QWidget* parent = nullptr);

      //! Sets the state to display to the user.
      void set_state(state state);

      //! Connects a slot to the login signal.
      boost::signals2::connection connect_login_signal(
        const login_signal::slot_type& slot) const;

      //! Connects a slot to the cancel signal.
      boost::signals2::connection connect_cancel_signal(
        const cancel_signal::slot_type& slot) const;

    private:
      mutable login_signal m_login_signal;
      mutable cancel_signal m_cancel_signal;
  };
}

#endif
