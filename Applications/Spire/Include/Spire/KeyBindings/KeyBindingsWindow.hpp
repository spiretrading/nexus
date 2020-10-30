#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include "Spire/KeyBindings/CancelKeyBindingsTableView.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/TaskKeyBindingsTableView.hpp"
#include "Spire/Ui/TabWidget.hpp"
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
        \param input_model The input model used for selecting order binding
                           securities.
        \param parent The parent widget.
      */
      explicit KeyBindingsWindow(KeyBindings key_bindings,
        Beam::Ref<SecurityInputModel> input_model, QWidget* parent = nullptr);

      //! Returns the current key bindings.
      const KeyBindings& get_key_bindings() const;

      //! Connects a slot to the applied signal.
      boost::signals2::connection connect_apply_signal(
        const ApplySignal::slot_type& slot) const;

    private:
      KeyBindings m_key_bindings;
      mutable ApplySignal m_apply_signal;
      TabWidget* m_tab_widget;
      bool m_last_focus_was_key;
      TaskKeyBindingsTableView* m_task_keys_table;
      CancelKeyBindingsTableView* m_cancel_keys_table;

      void on_ok_button_clicked();
      void on_restore_button_clicked();
  };
}

#endif
