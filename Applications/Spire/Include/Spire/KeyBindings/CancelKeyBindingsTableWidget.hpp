#ifndef SPIRE_CANCEL_KEY_BINDINGS_TABLE_WIDGET_HPP
#define SPIRE_CANCEL_KEY_BINDINGS_TABLE_WIDGET_HPP
#include <QTableWidget>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a table for editing cancellation key bindings.
  class CancelKeyBindingsTableWidget : public QTableWidget {
    public:

      //! Signals that a cancel key binding was modified.
      /*
        \param binding The modified cancel key binding.
      */
      using ModifiedSignal = Signal<void (
        const KeyBindings::CancelActionBinding& binding)>;

      //! Constructs a cancel key bindings table widget with initial
      //! key bindings.
      /*
        \param bindings The initial cancel key bindings.
        \param parent The parent widget.
      */
      explicit CancelKeyBindingsTableWidget(
        const std::vector<KeyBindings::CancelActionBinding>& bindings,
        QWidget* parent = nullptr);

      //! Sets the current cancel key bindings.
      /*
        \param bindings The current cancel key bindings.
      */
      void set_key_bindings(
        const std::vector<KeyBindings::CancelActionBinding>& bindings);

      //! Connects a slot to the modified signal.
      boost::signals2::connection connect_modified_signal(
        const ModifiedSignal::slot_type& slot) const;

    private:
      mutable ModifiedSignal m_modified_signal;
      std::vector<KeyBindings::CancelActionBinding> m_key_bindings;
  };
}

#endif
