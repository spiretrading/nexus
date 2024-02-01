#ifndef SPIRE_CONTROLLER_HPP
#define SPIRE_CONTROLLER_HPP
/** TODO */
#if 0
#include <memory>
#include <vector>
#include "Spire/Login/LoginController.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Controller for the application as a whole. */
  class SpireController {
    public:

      /**
       * Constructs a controller in a state ready to execute a new instance of
       * the application.
       */
      SpireController();

      ~SpireController();

      /** Begins running the application. */
      void open();

    private:
      enum class State {
        NONE,
        LOGIN,
        TOOLBAR
      };
      State m_state;
      std::unique_ptr<LoginController> m_login_controller;

      SpireController(const SpireController&) = delete;
      SpireController& operator =(const SpireController&) = delete;
      std::vector<LoginController::ServerEntry> load_server_entries();
      void on_login(const Definitions& definitions);
  };
}
#endif

#endif
