#include "Spire/KeyBindings/CancelKeyBindingsTableWidget.hpp"
#include <QHeaderView>
#include "Spire/Spire/Dimensions.hpp"
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
        return QObject::tr("All Bid");
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
  m_table = new QTableWidget(ROW_COUNT, COLUMN_COUNT, this);
  m_table->setFixedSize(scale(852, 368));
  m_table->setStyleSheet(QString(R"(
    QTableWidget {
      background-color: #FFFFFF;
      border: none;
      font-family: Roboto;
      font-size: %1px;
      gridline-color: #C8C8C8;
      padding-left: %2px;
      padding-right: %2px;
    }

    QTableWidget::item {
      padding-left: %2px;
    })").arg(scale_height(12)).arg(scale_width(8)));
  m_table->setHorizontalHeaderLabels({tr("Cancel Options"),
    tr("Key Bindings")});
  m_table->horizontalHeader()->installEventFilter(this);
  m_table->setFrameShape(QFrame::NoFrame);
  m_table->setFocusPolicy(Qt::NoFocus);
  m_table->setSelectionMode(QAbstractItemView::NoSelection);
  m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setColumnWidth(0, scale_width(238));
  m_table->horizontalHeader()->setFixedHeight(scale_height(30));
  m_table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft |
    Qt::AlignVCenter);
  m_table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft |
    Qt::AlignVCenter);
  m_table->horizontalHeader()->setSectionsMovable(false);
  m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->horizontalHeader()->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:/Icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)).arg(scale_height(12)));
  m_table->verticalHeader()->hide();
  m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  m_table->verticalHeader()->setDefaultSectionSize(scale_height(26));
  setWidget(m_table);
  set_key_bindings(bindings);
}

void CancelKeyBindingsTableWidget::set_key_bindings(
    const std::vector<KeyBindings::CancelActionBinding>& bindings) {
  m_key_bindings.clear();
  for(auto i = 0; i < ROW_COUNT; ++i) {
    m_key_bindings.push_back({{}, {}, get_action(i)});
    for(auto& binding : bindings) {
      
    }
    auto text_item = new QTableWidgetItem(get_action_text(
      m_key_bindings[i].m_action));
    text_item->setFlags(text_item->flags() & ~Qt::ItemIsEditable);
    m_table->setItem(i, 0, text_item);
  }
}

connection CancelKeyBindingsTableWidget::connect_modified_signal(
    const ModifiedSignal::slot_type& slot) const {
  return m_modified_signal.connect(slot);
}
