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
  const auto COLUMN_COUNT = 2;

  const auto ROW_COUNT = 13;

  auto get_action_text(KeyBindings::CancelAction action) {
    switch(action) {
      case KeyBindings::CancelAction::MOST_RECENT:
        return QObject::tr("Most Recent");
      case KeyBindings::CancelAction::MOST_RECENT_ASK:
        return QObject::tr("Most Recent Ask");
      case KeyBindings::CancelAction::MOST_RECENT_BID:
        return QObject::tr("Most Recent Bid");
      case KeyBindings::CancelAction::OLDEST:
        return QObject::tr("Oldest");
      case KeyBindings::CancelAction::OLDEST_ASK:
        return QObject::tr("Oldest Ask");
      case KeyBindings::CancelAction::OLDEST_BID:
        return QObject::tr("Oldest Bid");
      case KeyBindings::CancelAction::ALL:
        return QObject::tr("All");
      case KeyBindings::CancelAction::ALL_ASKS:
        return QObject::tr("All Asks");
      case KeyBindings::CancelAction::ALL_BIDS:
        return QObject::tr("All Bids");
      case KeyBindings::CancelAction::CLOSEST_ASK:
        return QObject::tr("Closest Ask");
      case KeyBindings::CancelAction::CLOSEST_BID:
        return QObject::tr("Closest Bid");
      case KeyBindings::CancelAction::FURTHEST_ASK:
        return QObject::tr("Furthest Ask");
      case KeyBindings::CancelAction::FURTHEST_BID:
        return QObject::tr("Furthest Bid");
      default:
        return QObject::tr("Invalid cancel action");
    }
  }

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
  setFixedWidth(scale_width(853));
  set_width(scale_width(853));
  //setFixedHeight(scale_height(338));
  set_column_width(0, scale_width(238));
  auto item_delegate = new KeySequenceItemDelegate(this);
  //item_delegate->connect_item_modified_signal([=] (auto row) {
  //  on_key_sequence_modified(row);
  //});
  set_column_delegate(1, item_delegate);
  //connect(m_table, &QTableWidget::cellClicked, [=] (auto row, auto column) {
  //  on_cell_clicked(row, column);
  //});
}

void CancelKeyBindingsTableView::set_key_bindings(
    const std::vector<KeyBindings::CancelActionBinding>& bindings) {
  set_model(new CancelKeyBindingsTableModel(bindings));
}

connection CancelKeyBindingsTableView::connect_modified_signal(
    const ModifiedSignal::slot_type& slot) const {
  return m_modified_signal.connect(slot);
}

//void CancelKeyBindingsTableWidget::on_cell_clicked(int row, int column) {
//  if(column == 1) {
//    auto index = m_table->model()->index(row, column);
//    m_table->edit(index);
//  }
//}
//
//void CancelKeyBindingsTableWidget::on_key_sequence_modified(int row) {
//  m_modified_signal({
//    m_table->item(row, 1)->data(Qt::DisplayRole).value<QKeySequence>(), {},
//      get_action(row)});
//}
