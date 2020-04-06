#include "Spire/KeyBindings/CancelKeyBindingsTableView.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/KeyBindings/KeySequenceItemDelegate.hpp"
#include "Spire/Ui/ItemPaddingDelegate.hpp"

using namespace boost::signals2;
using namespace Spire;
using ValidSequence = KeySequenceEditor::ValidKeySequence;

CancelKeyBindingsTableView::CancelKeyBindingsTableView(
    const std::vector<KeyBindings::CancelActionBinding>& bindings,
    QWidget* parent)
    : KeyBindingsTableView(make_fixed_header(parent), parent),
      m_table(nullptr) {
  set_key_bindings(bindings);
  setFixedWidth(scale_width(853));
  set_width(scale_width(853));
  set_height(scale_height(376));
  set_column_width(0, scale_width(238));
  auto valid_sequences = std::vector<std::vector<std::set<Qt::Key>>>(
    {std::vector<std::set<Qt::Key>>({{Qt::Key_Escape}}),
    std::vector<std::set<Qt::Key>>({{Qt::Key_Shift, Qt::Key_Alt, Qt::Key_ContrastAdjust}, {Qt::Key_Escape}})});
  auto item_delegate = new KeySequenceItemDelegate(valid_sequences, this);
  item_delegate->connect_item_modified_signal([=] (auto index) {
    on_key_sequence_modified(index);
  });
  set_column_delegate(1, item_delegate);
}

void CancelKeyBindingsTableView::set_key_bindings(
    const std::vector<KeyBindings::CancelActionBinding>& bindings) {
  m_table = new CancelKeyBindingsTableModel(bindings, this);
  set_model(m_table);
}

connection CancelKeyBindingsTableView::connect_modified_signal(
    const ModifiedSignal::slot_type& slot) const {
  return m_modified_signal.connect(slot);
}

void CancelKeyBindingsTableView::on_key_sequence_modified(
    const QModelIndex& index) {
  m_modified_signal({index.data(Qt::DisplayRole).value<QKeySequence>(), {},
    m_table->get_cancel_action(index.row())});
}
