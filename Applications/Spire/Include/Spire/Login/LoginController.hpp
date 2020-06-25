#ifndef SPIRE_LOGIN_CONTROLLER_HPP
#define SPIRE_LOGIN_CONTROLLER_HPP
#include <functional>
#include <memory>
#include <string>
#include <Beam/Network/IpAddress.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/ServiceClients.hpp"
#include "Spire/Login/Login.hpp"
#include "Spire/Spire/Definitions.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Signal.hpp"

namespace Spire {

  //! Allows the user to login to Spire.
  class LoginController : private boost::noncopyable {
    public:

      //! Specifies a server to connect to.
      struct ServerEntry {

        //! The name of the server.
        std::string m_name;

        //! The server's address.
        Beam::Network::IpAddress m_address;
      };

      //! Signals a successful login.
      /*!
        \param definitions The set of definitions available.
      */
      using LoggedInSignal = Signal<void (const Definitions& definitions)>;

      //! Factory used to build the Spire service clients.
      /*!
        \param username The username to login with.
        \param password The password to login with.
        \param address The IpAddress to connect to.
        \return The service clients used to login to Spire.
      */
      using ServiceClientsFactory = std::function<
        std::unique_ptr<Nexus::VirtualServiceClients>(
        const std::string& username, const std::string& password,
        const Beam::Network::IpAddress& address)>;

      //! Constructs a login controller in a state ready to display the login
      //! window.
      /*!
        \param servers The list of servers available to connect to.
        \param service_clients_factory Builds the service clients used to
               login to Spire.
      */
      LoginController(std::vector<ServerEntry> servers,
        ServiceClientsFactory service_clients_factory);

      ~LoginController();

      //! Returns the service clients that logged in.
      std::unique_ptr<Nexus::VirtualServiceClients>& get_service_clients();

      //! Launches the login window.
      void open();

      //! Connects a slot to the logged in signal.
      boost::signals2::connection connect_logged_in_signal(
        const LoggedInSignal::slot_type& slot) const;

    private:
      mutable LoggedInSignal m_logged_in_signal;
      std::vector<ServerEntry> m_servers;
      ServiceClientsFactory m_service_clients_factory;
      LoginWindow* m_login_window;
      QtPromise<std::unique_ptr<Nexus::VirtualServiceClients>> m_login_promise;
      std::unique_ptr<Nexus::VirtualServiceClients> m_service_clients;

      void on_login(const std::string& username, const std::string& password);
      void on_cancel();
      void on_login_promise(Beam::Expect<
        std::unique_ptr<Nexus::VirtualServiceClients>> service_clients);
  };
}

#endif
