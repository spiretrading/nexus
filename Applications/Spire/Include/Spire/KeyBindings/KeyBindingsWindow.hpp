#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  class KeyBindingsWindow : public Window {
    public:

      //! Signals that the user applied the current key bindings.
      /*
        /param key_bindings The applied key bindings.
      */
      using ApplySignal = Signal<void (const KeyBindings& binding)>;

      explicit KeyBindingsWindow(const KeyBindings& key_binding,
        QWidget* parent = nullptr);

      const KeyBindings& get_key_bindings() const;

      boost::signals2::connection connect_apply_signal(
        const ApplySignal::slot_type& slot) const;

    private:
      KeyBindings m_key_bindings;
      mutable ApplySignal m_apply_signal;
  };
}

#endif
