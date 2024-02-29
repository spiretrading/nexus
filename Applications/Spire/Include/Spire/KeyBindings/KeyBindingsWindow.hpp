#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the key bindings window. */
  class KeyBindingsWindow : public Window {
    public:

      /**
       * Constructs a KeyBindingsWindow.
       * @param parent The parent widget.
       */
      explicit KeyBindingsWindow(QWidget* parent = nullptr);

    private:
      void on_cancel();
      void on_done();
  };
}

#endif
