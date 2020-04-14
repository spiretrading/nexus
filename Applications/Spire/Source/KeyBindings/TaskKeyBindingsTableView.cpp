#include "Spire/KeyBindings/TaskKeyBindingsTableView.hpp"
#include "Spire/KeyBindings/InputFieldItemDelegate.hpp"
#include "Spire/KeyBindings/KeySequenceEditor.hpp"
#include "Spire/KeyBindings/KeySequenceItemDelegate.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;
using ValidSequence = KeySequenceEditor::ValidKeySequence;

TaskKeyBindingsTableView::TaskKeyBindingsTableView(
    std::vector<KeyBindings::OrderActionBinding> bindings,
    QWidget* parent)
    : KeyBindingsTableView(make_header(parent), parent),
      m_model(nullptr) {
  set_key_bindings(std::move(bindings));
  setFixedWidth(scale_width(853));
  set_width(scale_width(853));
  set_height(scale_height(376));
  set_column_width(0, scale_width(80));
  set_column_width(1, scale_width(80));
  set_column_width(2, scale_width(89));
  set_column_width(3, scale_width(89));
  set_column_width(4, scale_width(59));
  set_column_width(5, scale_width(80));
  set_column_width(6, scale_width(98));
  set_column_width(7, scale_width(144));
  auto valid_sequences = std::vector<std::vector<std::set<Qt::Key>>>(
    {ValidSequence({{Qt::Key_Escape}}),
    ValidSequence({{Qt::Key_Shift, Qt::Key_Alt, Qt::Key_Control},
    {Qt::Key_Escape}})});
  auto security_delegate = new InputFieldItemDelegate({"One", "Two", "Three"}, this);
  security_delegate->connect_item_modified_signal([=] (auto index) {
    on_item_modified(index);
  });
  set_column_delegate(1, security_delegate);
  auto key_delegate = new KeySequenceItemDelegate(valid_sequences, this);
  key_delegate->connect_item_modified_signal([=] (auto index) {
    on_item_modified(index);
  });
  set_column_delegate(8, key_delegate);
}

void TaskKeyBindingsTableView::set_key_bindings(
    const std::vector<KeyBindings::OrderActionBinding>& bindings) {
  m_model = new TaskKeyBindingsTableModel(bindings, this);
  set_model(m_model);
}

void TaskKeyBindingsTableView::on_item_modified(
    const QModelIndex& index) const {
  m_modified_signal(KeyBindings::OrderActionBinding{});
}
