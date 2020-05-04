#ifndef SPIRE_TASK_KEY_BINDINGS_TABLE_VIEW_HPP
#define SPIRE_TASK_KEY_BINDINGS_TABLE_VIEW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsTableView.hpp"
#include "Spire/KeyBindings/TaskKeyBindingsTableModel.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Displays a table for editing order action key bindings.
  class TaskKeyBindingsTableView : public KeyBindingsTableView {
    public:

      //! Signals that an order action key binding was modified.
      /*
        \param binding The modified order action binding.
      */
      using ModifiedSignal = Signal<void (
        const KeyBindings::OrderActionBinding& binding)>;

      //! Constructs a TaskKeyBindingsTableView.
      /*
        \param bindings The initial key bindings to display.
        \param input_model The input model for editing a binding's securtiy.
        \param parent The parent widget.
      */
      explicit TaskKeyBindingsTableView(
        std::vector<KeyBindings::OrderActionBinding> bindings,
        Beam::Ref<SecurityInputModel> input_model,
        QWidget* parent = nullptr);

      //! Sets the key bindings to display.
      /*
        \param bindings The key bindings to display.
      */
      void set_key_bindings(
        const std::vector<KeyBindings::OrderActionBinding>& bindings);

      //! Connects a slot to the modified signal.
      boost::signals2::connection connect_modified_signal(
        const ModifiedSignal::slot_type& slot) const;

    private:
      mutable ModifiedSignal m_modified_signal;
      std::vector<KeyBindings::OrderActionBinding> m_key_bindings;
      TaskKeyBindingsTableModel* m_model;

      bool is_valid(int row, int column) const;
      void on_item_modified(const QModelIndex& index);
      void on_row_count_changed();
  };
}

#endif
