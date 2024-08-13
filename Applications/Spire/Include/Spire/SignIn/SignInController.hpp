#ifndef SPIRE_SIGN_IN_CONTROLLER_HPP
#define SPIRE_SIGN_IN_CONTROLLER_HPP
#include <functional>
#include <memory>
#include <string>
#include <Beam/Network/IpAddress.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/SignIn/SignIn.hpp"

namespace Spire {

  /** Allows the user to sign in to Spire. */
  class SignInController {
    public:

      /** Specifies a server to connect to. */
      struct ServerEntry {

        /** The name of the server. */
        std::string m_name;

        /** The server's address. */
        Beam::Network::IpAddress m_address;
      };

      /**
       * Signals a successful sign in.
       * @param service_clients The ServiceClients that successfully signed in.
       */
      using SignedInSignal =
        Signal<void (const Nexus::ServiceClientsBox& service_clients)>;

      /**
       * Factory used to build the Spire service clients.
       * @param username The username to sign in with.
       * @param password The password to sign in with.
       * @param address The IpAddress to connect to.
       * @return The service clients used to sign in to Spire.
       */
      using ServiceClientsFactory = std::function<Nexus::ServiceClientsBox (
        const std::string& username, const std::string& password,
        const Beam::Network::IpAddress& address)>;

      /**
       * Constructs a sign in controller in a state ready to display the sign in
       * window.
       * @param version The application build version.
       * @param servers The list of servers available to connect to.
       * @param service_clients_factory Builds the service clients used to
       *        sign in to Spire.
       */
      SignInController(std::string version, std::vector<ServerEntry> servers,
        ServiceClientsFactory service_clients_factory);

      /** Launches the sign in window. */
      void open();

      /** Connects a slot to the signed in signal. */
      boost::signals2::connection connect_signed_in_signal(
        const SignedInSignal::slot_type& slot) const;

    private:
      mutable SignedInSignal m_signed_in_signal;
      std::string m_version;
      std::vector<ServerEntry> m_servers;
      ServiceClientsFactory m_service_clients_factory;
      SignInWindow* m_sign_in_window;
      QtPromise<void> m_sign_in_promise;
      bool m_run_update;

      SignInController(const SignInController&) = delete;
      SignInController& operator =(const SignInController&) = delete;
      void on_sign_in(const std::string& username, const std::string& password,
        Track track, const std::string& server);
      void on_cancel();
      void on_sign_in_promise(
        Beam::Expect<Nexus::ServiceClientsBox> service_clients);
  };
}

#endif
