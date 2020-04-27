#ifndef SPIRE_TASK_KEY_BINDINGS_TABLE_VIEW_HPP
#define SPIRE_TASK_KEY_BINDINGS_TABLE_VIEW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsTableView.hpp"
#include "Spire/KeyBindings/TaskKeyBindingsTableModel.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class TaskKeyBindingsTableView : public KeyBindingsTableView {
    public:

      using ModifiedSignal = Signal<void (
        const KeyBindings::OrderActionBinding& binding)>;

      explicit TaskKeyBindingsTableView(
        std::vector<KeyBindings::OrderActionBinding> bindings,
        Beam::Ref<SecurityInputModel> input_model,
        QWidget* parent = nullptr);

      void set_key_bindings(
        const std::vector<KeyBindings::OrderActionBinding>& bindings);

    private:
      mutable ModifiedSignal m_modified_signal;
      std::vector<KeyBindings::OrderActionBinding> m_key_bindings;
      TaskKeyBindingsTableModel* m_model;

      bool is_valid(int row, int column) const;
      void on_item_modified(const QModelIndex& index) const;
      void on_row_count_changed();
  };
}

#endif
