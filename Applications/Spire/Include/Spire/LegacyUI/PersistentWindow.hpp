#ifndef SPIRE_PERSISTENT_WINDOW_HPP
#define SPIRE_PERSISTENT_WINDOW_HPP
#include <memory>
#include "Spire/LegacyUI/LegacyUI.hpp"

namespace Spire::LegacyUI {

  /** Interface for a window that persists between sessions. */
  class PersistentWindow {
    public:
      virtual ~PersistentWindow() = default;

      /** Returns this window's settings. */
      virtual std::unique_ptr<WindowSettings> GetWindowSettings() const = 0;
  };
}

#endif
