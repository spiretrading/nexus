#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the key bindings window. */
  class KeyBindingsWindow : public Window {
    public:

      /**
       * Constructs a KeyBindingsWindow.
       * @param key_bindings The KeyBindingsModel to display.
       * @param securities The set of securities to use.
       * @param countries The country database to use.
       * @param markets The market database to use.
       * @param destinations The destination database to use.
       * @param additional_tags Defines all available additional tags.
       * @param parent The parent widget.
       */
      KeyBindingsWindow(std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<ComboBox::QueryModel> securities,
        const Nexus::CountryDatabase& countries,
        const Nexus::MarketDatabase& markets,
        const Nexus::DestinationDatabase& destinations,
        const AdditionalTagDatabase& additional_tags,
        QWidget* parent = nullptr);

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;

      void on_cancel();
      void on_done();
  };
}

#endif
