#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorContextMenu.hpp"
#include <QEvent>
#include <QWidgetAction>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/DropShadow.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto CONTEXT_MENU_SIZE() {
    return scale(140, 90);
  }

  auto SUB_MENU_SIZE() {
    return scale(140, 130);
  }

  auto CHECK_BOX_SIZE() {
    return scale(140, 20);
  }
}

OrderImbalanceIndicatorContextMenu::OrderImbalanceIndicatorContextMenu(
    QWidget* parent)
    : QMenu(parent) {
  setWindowFlag(Qt::NoDropShadowWindowHint);
  m_drop_shadow = new DropShadow(true, true, this);
  setFixedSize(CONTEXT_MENU_SIZE());
  setStyleSheet(QString(R"(
    QMenu {
      background-color: #FFFFFF;
      border: %1px solid #A0A0A0 %2px solid #A0A0A0;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
      padding: %4px 0px;
    }

    QMenu::item {
      padding: %5px 0px %5px %6px;
    }

    QMenu::item:disabled,
    QMenu::item:disabled:selected,
    QMenu::item:disabled:hover {
      background-color: #FFFFFF;
      color: #C8C8C8;
    }

    QMenu::right-arrow {
      height: %7px;
      image: url(:/Icons/arrow-right-black.svg);
      padding-right: %6px;
      width: %8px;
    }

    QMenu::right-arrow:selected {
      height: %7px;
      image: url(:/Icons/arrow-right-white.svg);
      padding-right: %6px;
      width: %8px;
    }

    QMenu::item:selected, QMenu::item:hover {
      background-color: #8D78EC;
      color: #FFFFFF;
    })")
    .arg(scale_height(1)).arg(scale_width(1))
    .arg(scale_height(12)).arg(scale_height(5))
    .arg(scale_height(3)).arg(scale_width(8))
    .arg(scale_height(6)).arg(scale_width(4)));
  m_table_columns_menu = new QMenu(tr("Table Columns"), this);
  m_table_columns_menu->setWindowFlag(Qt::NoDropShadowWindowHint);
  m_sub_menu_drop_shadow = new DropShadow(false, true, m_table_columns_menu);
  m_table_columns_menu->installEventFilter(this);
  m_table_columns_menu->setFixedSize(SUB_MENU_SIZE());
  m_table_columns_menu->setContentsMargins(0, scale_height(0), 0,
    scale_height(4));
  addMenu(m_table_columns_menu);
  add_checkbox(tr("Side"), m_side_signal);
  add_checkbox(tr("Size"), m_size_signal);
  add_checkbox(tr("Reference Px"), m_reference_price_signal);
  add_checkbox(tr("Date"), m_date_signal);
  add_checkbox(tr("Time"), m_time_signal);
  add_checkbox(tr("Notional Value"), m_notional_value_signal);
  add_menu_item(tr("Export Table"), m_export_table_signal);
  add_menu_item(tr("Export Chart"), m_export_chart_signal);
  add_menu_item(tr("Reset All Filters"), m_reset_signal);
}

connection OrderImbalanceIndicatorContextMenu::connect_export_table_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_export_table_signal.connect(slot);
}

connection OrderImbalanceIndicatorContextMenu::connect_export_chart_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_export_chart_signal.connect(slot);
}

connection OrderImbalanceIndicatorContextMenu::connect_reset_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_reset_signal.connect(slot);
}

connection OrderImbalanceIndicatorContextMenu::connect_side_toggled_signal(
    const ToggledSignal::slot_type& slot) const {
  return m_side_signal.connect(slot);
}

connection OrderImbalanceIndicatorContextMenu::connect_size_toggled_signal(
    const ToggledSignal::slot_type& slot) const {
  return m_size_signal.connect(slot);
}

connection
    OrderImbalanceIndicatorContextMenu::connect_reference_price_toggled_signal(
    const ToggledSignal::slot_type& slot) const {
  return m_reference_price_signal.connect(slot);
}

connection OrderImbalanceIndicatorContextMenu::connect_date_toggled_signal(
    const ToggledSignal::slot_type& slot) const {
  return m_date_signal.connect(slot);
}

connection OrderImbalanceIndicatorContextMenu::connect_time_toggled_signal(
    const ToggledSignal::slot_type& slot) const {
  return m_time_signal.connect(slot);
}

connection
    OrderImbalanceIndicatorContextMenu::connect_notional_value_toggled_signal(
    const ToggledSignal::slot_type& slot) const {
  return m_notional_value_signal.connect(slot);
}

bool OrderImbalanceIndicatorContextMenu::eventFilter(QObject* watched,
    QEvent* event) {
  if(watched == m_table_columns_menu) {
    if(event->type() == QEvent::Show) {
      m_table_columns_menu->move(pos().x() + scale_width(136),
        pos().y() + scale_height(6));
    }
  }
  return QMenu::eventFilter(watched, event);
}

void OrderImbalanceIndicatorContextMenu::add_checkbox(const QString& text,
    ToggledSignal& signal) {
  auto checkbox = new Checkbox(text, this);
  checkbox->setFixedSize(CHECK_BOX_SIZE());
  checkbox->setChecked(true);
  checkbox->setLayoutDirection(Qt::RightToLeft);
  auto action = new QWidgetAction(this);
  action->setDefaultWidget(checkbox);
  m_table_columns_menu->addAction(action);
  connect(checkbox, &Checkbox::stateChanged,
    [signal = &signal] (int state) {
      (*signal)(state == Qt::Checked);
    });
}

void OrderImbalanceIndicatorContextMenu::add_menu_item(const QString& text,
    SelectedSignal& signal) {
  auto action = new QAction(text, this);
  connect(action, &QAction::triggered, [signal = &signal] { (*signal)(); });
  addAction(action);
}
