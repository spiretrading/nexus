#ifndef SPIRE_CANCEL_KEY_BINDINGS_TABLE_VIEW_HPP
#define SPIRE_CANCEL_KEY_BINDINGS_TABLE_VIEW_HPP
#include <QTableWidget>
#include "Spire/KeyBindings/CancelKeyBindingsTableModel.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsTableView.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Displays a table for editing cancel action key bindings.
  class CancelKeyBindingsTableView : public KeyBindingsTableView {
    public:

      //! Signals that a cancel action key binding was modified.
      /*
        \param binding The modified cancel key binding.
      */
      using ModifiedSignal = Signal<void (
        const KeyBindings::CancelActionBinding& binding)>;

      //! Constructs a cancel action key bindings table widget with initial
      //! key bindings.
      /*
        \param bindings The initial cancel key bindings.
        \param parent The parent widget.
      */
      explicit CancelKeyBindingsTableView(
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
      CancelKeyBindingsTableModel* m_model;

      void on_key_sequence_modified(const QModelIndex& index);
  };
}

#endif
