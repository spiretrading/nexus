#ifndef SPIRE_CONTROLLER_HPP
#define SPIRE_CONTROLLER_HPP
#include <memory>
#include <vector>
#include <Beam/Network/Network.hpp>
#include <Beam/Threading/Threading.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/login/login_controller.hpp"
#include "spire/spire/spire.hpp"
#include "spire/toolbar/toolbar.hpp"

namespace Spire {

  //! Controller for the application as a whole.
  class SpireController : private boost::noncopyable {
    public:

      //! Constructs a controller in a state ready to execute a new instance of
      //! the application.
      SpireController();

      ~SpireController();

      //! Begins running the application.
      void open();

    private:
      enum class State {
        NONE,
        LOGIN,
        TOOLBAR
      };
      State m_state;
      std::unique_ptr<Beam::Network::SocketThreadPool> m_socket_thread_pool;
      std::unique_ptr<Beam::Threading::TimerThreadPool> m_timer_thread_pool;
      std::unique_ptr<LoginController> m_login_controller;
      std::unique_ptr<Nexus::VirtualServiceClients> m_service_clients;
      std::unique_ptr<ToolbarController> m_toolbar_controller;

      std::vector<LoginController::ServerEntry> load_server_entries();
      void on_login();
  };
}

#endif
