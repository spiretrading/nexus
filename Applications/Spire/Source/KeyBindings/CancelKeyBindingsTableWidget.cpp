#include "Spire/KeyBindings/CancelKeyBindingsTableWidget.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
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

CancelKeyBindingsTableWidget::CancelKeyBindingsTableWidget(
    const std::vector<KeyBindings::CancelActionBinding>& bindings,
    QWidget* parent)
    : ScrollArea(true, parent) {
  auto main_widget = new QWidget(this);
  main_widget->setFixedSize(scale(850, 338));
  auto layout = new QVBoxLayout(main_widget);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto header_widget = new QWidget(this);
  auto header_layout = new QHBoxLayout(header_widget);
  header_layout->setContentsMargins(scale_width(8), 0, 0, 0);
  header_layout->setSpacing(0);
  layout->addLayout(header_layout);
  auto options_label = new QLabel(tr("Cancel Options"), this);
  options_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  auto label_style = QString(R"(
      background-color: #FFFFFF;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %1px;
      font-weight: 550;
    )").arg(scale_height(12));
  options_label->setStyleSheet(label_style);
  options_label->setFixedSize(scale(238, 30));
  header_layout->addWidget(options_label);
  auto bindings_label = new QLabel(tr("Key Binding"), this);
  bindings_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  bindings_label->setStyleSheet(label_style);
  bindings_label->setFixedSize(scale(606, 30));
  header_layout->addWidget(bindings_label);
  m_table = new CustomGridTableWidget(ROW_COUNT, COLUMN_COUNT, this);
  layout->addWidget(m_table);
  m_table->setFixedSize(scale(852, 338));
  setWidget(main_widget);
  auto item_delegate = new KeySequenceItemDelegate(this);
  item_delegate->connect_item_modified_signal([=] (auto row) {
    on_key_sequence_modified(row);
  });
  m_table->setItemDelegateForColumn(1, item_delegate);
  m_table->setEditTriggers(QAbstractItemView::EditTrigger::SelectedClicked);
  connect(m_table, &QTableWidget::cellClicked, [=] (auto row, auto column) {
    on_cell_clicked(row, column);
  });
  set_key_bindings(bindings);
}

void CancelKeyBindingsTableWidget::set_key_bindings(
    const std::vector<KeyBindings::CancelActionBinding>& bindings) {
  m_key_bindings.clear();
  for(auto i = 0; i < ROW_COUNT; ++i) {
    m_key_bindings.push_back({{}, {}, get_action(i)});
    for(auto& binding : bindings) {
      auto iter = std::find_if(bindings.begin(), bindings.end(),
        [&] (auto item) {
          return m_key_bindings.back().m_action == item.m_action;
        });
      if(iter != bindings.end()) {
        m_key_bindings.back().m_sequence = binding.m_sequence;
      }
    }
    auto text_item = new QTableWidgetItem(get_action_text(
      m_key_bindings[i].m_action));
    text_item->setFlags(text_item->flags() & ~Qt::ItemIsEditable);
    m_table->setItem(i, 0, text_item);
    auto key_item = new QTableWidgetItem();
    key_item->setData(Qt::DisplayRole, QVariant::fromValue<QKeySequence>(
      m_key_bindings.back().m_sequence));
    m_table->setItem(i, 1, key_item);
  }
}

connection CancelKeyBindingsTableWidget::connect_modified_signal(
    const ModifiedSignal::slot_type& slot) const {
  return m_modified_signal.connect(slot);
}

void CancelKeyBindingsTableWidget::on_cell_clicked(int row, int column) {
  if(column == 1) {
    auto index = m_table->model()->index(row, column);
    m_table->edit(index);
  }
}

void CancelKeyBindingsTableWidget::on_key_sequence_modified(int row) {
  m_modified_signal({
    m_table->item(row, 1)->data(Qt::DisplayRole).value<QKeySequence>(), {},
      get_action(row)});
}
