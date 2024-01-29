#ifndef SPIRE_LOGIN_CONTROLLER_HPP
#define SPIRE_LOGIN_CONTROLLER_HPP
#include <functional>
#include <memory>
#include <string>
#include <Beam/Network/IpAddress.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/Login/Login.hpp"
#include "Spire/Spire/Definitions.hpp"

namespace Spire {

  /** Allows the user to login to Spire. */
  class LoginController {
    public:

      /** Specifies a server to connect to. */
      struct ServerEntry {

        /** The name of the server. */
        std::string m_name;

        /** The server's address. */
        Beam::Network::IpAddress m_address;
      };

      /**
       * Signals a successful login.
       * @param definitions The set of definitions available.
       */
      using LoggedInSignal = Signal<void (const Definitions& definitions)>;

      /**
       * Factory used to build the Spire service clients.
       * @param username The username to login with.
       * @param password The password to login with.
       * @param address The IpAddress to connect to.
       * @return The service clients used to login to Spire.
       */
      using ServiceClientsFactory = std::function<Nexus::ServiceClientsBox (
        const std::string& username, const std::string& password,
        const Beam::Network::IpAddress& address)>;

      /**
       * Constructs a login controller in a state ready to display the login
       * window.
       * @param version The application build version.
       * @param servers The list of servers available to connect to.
       * @param service_clients_factory Builds the service clients used to
       *        login to Spire.
       */
      LoginController(std::string version, std::vector<ServerEntry> servers,
        ServiceClientsFactory service_clients_factory);

      /** Returns the service clients that logged in. */
      Nexus::ServiceClientsBox get_service_clients();

      /** Launches the login window. */
      void open();

      /** Connects a slot to the logged in signal. */
      boost::signals2::connection connect_logged_in_signal(
        const LoggedInSignal::slot_type& slot) const;

    private:
      mutable LoggedInSignal m_logged_in_signal;
      std::string m_version;
      std::vector<ServerEntry> m_servers;
      ServiceClientsFactory m_service_clients_factory;
      LoginWindow* m_login_window;
      QtPromise<void> m_login_promise;
      boost::optional<Nexus::ServiceClientsBox> m_service_clients;

      LoginController(const LoginController&) = delete;
      LoginController& operator =(const LoginController&) = delete;
      void on_login(const std::string& username, const std::string& password);
      void on_cancel();
      void on_login_promise(
        Beam::Expect<Nexus::ServiceClientsBox> service_clients);
  };
}

#endif
