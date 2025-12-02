#ifndef SPIRE_LOGIN_DIALOG_HPP
#define SPIRE_LOGIN_DIALOG_HPP
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <QDialog>
#include "Spire/Async/QtPromise.hpp"
#include "Spire/UI/UI.hpp"

class Ui_LoginDialog;

namespace Spire::UI {

  /** Presents the login dialog. */
  class LoginDialog : public QDialog {
    public:

      /** Specifies a server to connect to. */
      struct ServerEntry {

        /** The name of the server. */
        std::string m_name;

        /** The server's address. */
        Beam::IpAddress m_address;
      };

      /**
       * Constructs the LoginDialog.
       * @param servers The list of servers available to connect to.
       */
      explicit LoginDialog(std::vector<ServerEntry> servers);

      ~LoginDialog() override;

      /** Returns the ServiceLocatorClient that logged in. */
      std::unique_ptr<Beam::ApplicationServiceLocatorClient>
        GetServiceLocatorClient();

      /** Returns the username. */
      std::string GetUsername() const;

      /** Returns the password. */
      std::string GetPassword() const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;

    private:
      enum class State {
        READY,
        LOADING
      };
      std::unique_ptr<Ui_LoginDialog> m_ui;
      std::vector<ServerEntry> m_servers;
      std::unique_ptr<Beam::ApplicationServiceLocatorClient>
        m_serviceLocatorClient;
      State m_state;
      std::optional<QtPromise<void>> m_loginPromise;

      void UpdateChromaHash();
      void OnPasswordTextChanged(const QString& text);
      void OnLoginButtonClicked();
      void OnCancelButtonClicked();
  };
}

#endif
