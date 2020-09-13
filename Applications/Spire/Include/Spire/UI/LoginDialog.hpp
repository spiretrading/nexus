#ifndef SPIRE_LOGIN_DIALOG_HPP
#define SPIRE_LOGIN_DIALOG_HPP
#include <memory>
#include <string>
#include <vector>
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

namespace Spire::UI {

  /** Presents the login dialog. */
  class LoginDialog : public QDialog {
    public:

      //! Specifies a server to connect to.
      struct ServerEntry {

        //! The name of the server.
        std::string m_name;

        //! The server's address.
        Beam::Network::IpAddress m_address;
      };

      //! Constructs the LoginDialog.
      /*!
        \param servers The list of servers available to connect to.
      */
      LoginDialog(std::vector<ServerEntry> servers);

      virtual ~LoginDialog();

      //! Returns the ServiceLocatorClient that logged in.
      std::unique_ptr<Beam::ServiceLocator::ApplicationServiceLocatorClient>
        GetServiceLocatorClient();

      //! Returns the username.
      std::string GetUsername() const;

      //! Returns the password.
      std::string GetPassword() const;

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      struct LoginRoutine;
      std::unique_ptr<Ui_LoginDialog> m_ui;
      std::vector<ServerEntry> m_servers;
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

#endif
