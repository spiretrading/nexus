#ifndef SPIRE_PERSISTENTWINDOW_HPP
#define SPIRE_PERSISTENTWINDOW_HPP
#include <memory>
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class PersistentWindow
      \brief Interface for a window that persists between sessions.
   */
  class PersistentWindow {
    public:

      virtual ~PersistentWindow();

      //! Returns this window's settings.
      virtual std::unique_ptr<WindowSettings> GetWindowSettings() const = 0;
  };
}
}

#endif
