#ifndef SPIRE_SIGN_IN_CONTROLLER_HPP
#define SPIRE_SIGN_IN_CONTROLLER_HPP
#include <bitset>
#include <functional>
#include <memory>
#include <string>
#include <Beam/Network/IpAddress.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Clients/Clients.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/SignIn/Track.hpp"
#include "Spire/Ui/ProgressBar.hpp"

namespace Spire {
  class SignInWindow;

  /** Allows the user to sign in to Spire. */
  class SignInController {
    public:

      /** Specifies a server to connect to. */
      struct ServerEntry {

        /** The name of the server. */
        std::string m_name;

        /** The server's address. */
        Beam::IpAddress m_address;
      };

      /**
       * Signals a successful sign in.
       * @param clients The Clients that successfully signed in.
       */
      using SignedInSignal = Signal<void (const Nexus::Clients& clients)>;

      /**
       * Factory used to build the Spire clients.
       * @param username The username to sign in with.
       * @param password The password to sign in with.
       * @param address The IpAddress to connect to.
       * @return The clients used to sign in to Spire.
       */
      using ClientsFactory =
        std::function<Nexus::Clients (const std::string& username,
          const std::string& password, const Beam::IpAddress& address)>;

      /**
       * Constructs a sign in controller in a state ready to display the sign in
       * window.
       * @param version The application build version.
       * @param servers The list of servers available to connect to.
       * @param clients_factory Builds the clients used to sign in to Spire.
       */
      SignInController(std::string version, std::vector<ServerEntry> servers,
        ClientsFactory clients_factory);

      /** Launches the sign in window. */
      void open();

      /** Connects a slot to the signed in signal. */
      boost::signals2::connection connect_signed_in_signal(
        const SignedInSignal::slot_type& slot) const;

    private:
      mutable SignedInSignal m_signed_in_signal;
      std::string m_version;
      std::vector<ServerEntry> m_servers;
      ClientsFactory m_clients_factory;
      std::shared_ptr<ProgressModel> m_download_progress;
      std::shared_ptr<ProgressModel> m_installation_progress;
      std::shared_ptr<ValueModel<boost::posix_time::time_duration>> m_time_left;
      SignInWindow* m_sign_in_window;
      QtPromise<void> m_sign_in_promise;
      std::bitset<3> m_run_update;

      SignInController(const SignInController&) = delete;
      SignInController& operator =(const SignInController&) = delete;
      void on_sign_in(const std::string& username, const std::string& password,
        Track track, const std::string& server);
      void on_cancel();
      void on_sign_in_promise(Beam::Expect<Nexus::Clients> clients);
  };
}

#endif
