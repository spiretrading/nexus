#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the key bindings window. */
  class KeyBindingsWindow : public Window {
    public:

      /**
       * Constructs a KeyBindingsWindow.
       * @param key_bindings The KeyBindingsModel to display.
       * @param securities The set of securities to use.
       * @param additional_tags Defines all available additional tags.
       * @param parent The parent widget.
       */
      KeyBindingsWindow(std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<SecurityInfoQueryModel> securities,
        const AdditionalTagDatabase& additional_tags,
        QWidget* parent = nullptr);

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;

      void on_cancel();
      void on_done();
  };
}

#endif
