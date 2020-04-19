#ifndef SPIRE_KEY_BINDINGS_WINDOW_HPP
#define SPIRE_KEY_BINDINGS_WINDOW_HPP
#include <QTabWidget>
#include "Spire/KeyBindings/CancelKeyBindingsTableView.hpp"
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
      explicit KeyBindingsWindow(KeyBindings key_bindings,
        QWidget* parent = nullptr);

      //! Returns the current key bindings.
      const KeyBindings& get_key_bindings() const;

      //! Connects a slot to the applied signal.
      boost::signals2::connection connect_apply_signal(
        const ApplySignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      KeyBindings m_key_bindings;
      mutable ApplySignal m_apply_signal;
      QTabWidget* m_tab_widget;
      bool m_last_focus_was_key;
      CancelKeyBindingsTableView* m_cancel_keys_table;

      void on_ok_button_clicked();
      void on_restore_button_clicked();
      void on_tab_bar_clicked(int index);
      void on_tab_changed();
  };
}

#endif
