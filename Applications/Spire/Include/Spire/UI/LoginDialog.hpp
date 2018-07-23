#ifndef SPIRE_LOGINDIALOG_HPP
#define SPIRE_LOGINDIALOG_HPP
#include <memory>
#include <string>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/Network.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/Threading/Sync.hpp>
#include <QDialog>
#include <QTimer>
#include "Spire/UI/UI.hpp"

class Ui_LoginDialog;

namespace Spire {
namespace UI {

  /*! \class LoginDialog
      \brief Presents the login dialog.
   */
  class LoginDialog : public QDialog {
    public:

      //! Constructs the LoginDialog.
      /*!
        \param address The IP address to connect to.
        \param username The initial username to display.
        \param saveLoginInfo Whether the save login box is initially checked.
        \param socketThreadPool The SocketThreadPool to use for socket
               connections.
        \param timerThreadPool The TimerThreadPool to use for heartbeats.
      */
      LoginDialog(const Beam::Network::IpAddress& address,
        const std::string& username, bool saveLoginInfo,
        Beam::RefType<Beam::Network::SocketThreadPool> socketThreadPool,
        Beam::RefType<Beam::Threading::TimerThreadPool> timerThreadPool);

      virtual ~LoginDialog();

      //! Returns the ServiceLocatorClient that logged in.
      std::unique_ptr<Beam::ServiceLocator::ApplicationServiceLocatorClient>
        GetServiceLocatorClient();

      //! Returns the username.
      std::string GetUsername() const;

      //! Returns the password.
      std::string GetPassword() const;

      //! Returns <code>true</code> iff the save login info box is checked.
      bool IsSaveLoginInfoChecked() const;

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      struct LoginRoutine;
      std::unique_ptr<Ui_LoginDialog> m_ui;
      Beam::Network::IpAddress m_address;
      std::string m_username;
      bool m_saveLoginInfo;
      Beam::Network::SocketThreadPool* m_socketThreadPool;
      Beam::Threading::TimerThreadPool* m_timerThreadPool;
      std::unique_ptr<Beam::ServiceLocator::ApplicationServiceLocatorClient>
        m_serviceLocatorClient;
      std::unique_ptr<Beam::Threading::Sync<LoginRoutine>> m_loginRoutine;
      enum {
        READY,
        LOADING,
      } m_state;
      QTimer m_updateTimer;

      void UpdateChromaHash();
      void OnPasswordTextChanged(const QString& text);
      void OnLoginButtonClicked();
      void OnCancelButtonClicked();
      void OnUpdateTimer();
  };
}
}

#endif
