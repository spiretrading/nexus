#ifndef SPIRE_TASK_KEY_BINDINGS_TABLE_VIEW_HPP
#define SPIRE_TASK_KEY_BINDINGS_TABLE_VIEW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsTableView.hpp"
#include "Spire/KeyBindings/TaskKeyBindingsTableModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class TaskKeyBindingsTableView : public KeyBindingsTableView {
    public:

      explicit TaskKeyBindingsTableView(
        std::vector<KeyBindings::OrderActionBinding> bindings,
        QWidget* parent = nullptr);

      void set_key_bindings(
        const std::vector<KeyBindings::OrderActionBinding>& bindings);

    private:
      std::vector<KeyBindings::OrderActionBinding> m_key_bindings;
      TaskKeyBindingsTableModel* m_model;
  };
}

#endif
