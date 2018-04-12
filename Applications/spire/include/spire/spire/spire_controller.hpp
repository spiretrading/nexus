#ifndef SPIRE_CONTROLLER_HPP
#define SPIRE_CONTROLLER_HPP
#include <memory>
#include <Beam/Network/Network.hpp>
#include <Beam/Threading/Threading.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/login/login.hpp"
#include "spire/spire/spire.hpp"
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! Controller for the application as a whole.
  class spire_controller : private boost::noncopyable {
    public:

      //! Constructs a controller in a state ready to execute a new instance of
      //! the application.
      spire_controller();

      ~spire_controller();

      //! Begins running the application.
      void open();

    private:
      enum class state {
        NONE,
        LOGIN,
        TOOLBAR
      };
      state m_state;
      std::unique_ptr<Beam::Network::SocketThreadPool> m_socket_thread_pool;
      std::unique_ptr<Beam::Threading::TimerThreadPool> m_timer_thread_pool;
      std::unique_ptr<login_controller> m_login_controller;
      std::unique_ptr<Nexus::VirtualServiceClients> m_service_clients;
      std::unique_ptr<toolbar_controller> m_toolbar_controller;

      boost::optional<Beam::Network::IpAddress> load_ip_address();
      void on_login();
  };
}

#endif
