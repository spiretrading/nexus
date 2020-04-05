#include "Spire/KeyBindings/CancelKeyBindingsTableView.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsTableModel.hpp"
#include "Spire/KeyBindings/KeySequenceItemDelegate.hpp"
#include "Spire/Ui/ItemPaddingDelegate.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto get_action(int index) {
    static auto actions = std::vector<KeyBindings::CancelAction>({
      KeyBindings::CancelAction::ALL,
      KeyBindings::CancelAction::ALL_ASKS,
      KeyBindings::CancelAction::ALL_BIDS,
      KeyBindings::CancelAction::CLOSEST_ASK,
      KeyBindings::CancelAction::CLOSEST_BID,
      KeyBindings::CancelAction::FURTHEST_BID,
      KeyBindings::CancelAction::FURTHEST_ASK,
      KeyBindings::CancelAction::MOST_RECENT,
      KeyBindings::CancelAction::MOST_RECENT_ASK,
      KeyBindings::CancelAction::MOST_RECENT_BID,
      KeyBindings::CancelAction::OLDEST,
      KeyBindings::CancelAction::OLDEST_ASK,
      KeyBindings::CancelAction::OLDEST_BID
    });
    return actions[index];
  }
}

CancelKeyBindingsTableView::CancelKeyBindingsTableView(
    const std::vector<KeyBindings::CancelActionBinding>& bindings,
    QWidget* parent)
    : KeyBindingsTableView(make_fixed_header(parent), parent) {
  set_key_bindings(bindings);
  set_width(scale_width(853));
  set_height(scale_height(338));
  set_column_width(0, scale_width(238));
  auto item_delegate = new KeySequenceItemDelegate(this);
  item_delegate->connect_item_modified_signal([=] (auto index) {
    on_key_sequence_modified(index);
  });
  set_column_delegate(1, item_delegate);
}

void CancelKeyBindingsTableView::set_key_bindings(
    const std::vector<KeyBindings::CancelActionBinding>& bindings) {
  set_model(new CancelKeyBindingsTableModel(bindings));
}

connection CancelKeyBindingsTableView::connect_modified_signal(
    const ModifiedSignal::slot_type& slot) const {
  return m_modified_signal.connect(slot);
}

void CancelKeyBindingsTableView::on_key_sequence_modified(
    const QModelIndex& index) {
  m_modified_signal({index.data(Qt::DisplayRole).value<QKeySequence>(), {},
    get_action(index.row())});
}
