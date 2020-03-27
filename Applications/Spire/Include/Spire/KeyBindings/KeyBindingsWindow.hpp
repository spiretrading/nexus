#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  //! Displays the user's key bindings.
  class KeyBindingsWindow : public Window {
    public:

      //! Signals that the user applied the current key bindings.
      using ApplySignal = Signal<void ()>;

      //! Constructs a key bindings window.
      /*
        \param key_bindings The initial key bindings.
        \param parent The parent widget.
      */
      explicit KeyBindingsWindow(const KeyBindings& key_bindings,
        QWidget* parent = nullptr);

      //! Returns the current key bindings.
      const KeyBindings& get_key_bindings() const;

      //! Connects a slot to the applied signal.
      boost::signals2::connection connect_apply_signal(
        const ApplySignal::slot_type& slot) const;

    private:
      KeyBindings m_key_bindings;
      mutable ApplySignal m_apply_signal;
  };
}

#endif
