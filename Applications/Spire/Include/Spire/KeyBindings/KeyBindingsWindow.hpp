#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the key bindings window. */
  class KeyBindingsWindow : public Window {
    public:

      /**
       * Constructs a KeyBindingsWindow.
       * @param key_bindings The KeyBindingsModel to display.
       * @param parent The parent widget.
       */
      explicit KeyBindingsWindow(std::shared_ptr<KeyBindingsModel> key_bindings,
        const Nexus::CountryDatabase& countries,
        const Nexus::MarketDatabase& markets, QWidget* parent = nullptr);

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;

      void on_cancel();
      void on_done();
  };
}

#endif
