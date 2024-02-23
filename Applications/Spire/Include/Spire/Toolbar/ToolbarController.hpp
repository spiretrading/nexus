#ifndef SPIRE_TOOLBAR_CONTROLLER_HPP
#define SPIRE_TOOLBAR_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

namespace Spire {

  /** Implements the main application controller for the ToolbarWindow. */
  class ToolbarController {
    public:

      /**
       * Constructs a ToolbarController.
       * @param user_profile The user's profile.
       */
      explicit ToolbarController(Beam::Ref<UserProfile> user_profile);

      /** Displays the toolbar window. */
      void open();

      /** Closes the toolbar window and all associated windows. */
      void close();

    private:
      UserProfile* m_user_profile;
      std::unique_ptr<ToolbarWindow> m_toolbar_window;

      ToolbarController(const ToolbarController&) = delete;
      ToolbarController& operator =(const ToolbarController&) = delete;
  };
}

#endif
