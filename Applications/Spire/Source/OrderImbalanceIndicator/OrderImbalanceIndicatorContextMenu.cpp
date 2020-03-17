#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorContextMenu.hpp"
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QWidgetAction>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/DropShadow.hpp"

using namespace Spire;

OrderImbalanceIndicatorContextMenu::OrderImbalanceIndicatorContextMenu(
    QWidget* parent)
    : QMenu(parent) {
  setWindowFlag(Qt::NoDropShadowWindowHint);
  DropShadow context_menu_shadow(true, true, this);
  setFixedSize(scale(140, 90));
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

    QMenu::item:selected, QMenu::item:hover {
      background-color: #8D78EC;
      color: #FFFFFF;
    })")
    .arg(scale_height(1)).arg(scale_width(1))
    .arg(scale_height(12)).arg(scale_height(5))
    .arg(scale_height(3)).arg(scale_width(8)));
  m_table_columns_menu = new QMenu(tr("Table Columns"), this);
  m_table_columns_menu->installEventFilter(this);
  m_table_columns_menu->setFixedSize(scale(140, 150));
  m_table_columns_menu->setContentsMargins(0, scale_height(0), 0,
    scale_height(4));
  addMenu(m_table_columns_menu);
  auto security_action = new QWidgetAction(this);
  add_check_box(m_security_check_box, tr("Security"), security_action);
  m_table_columns_menu->addAction(security_action);
  auto side_action = new QWidgetAction(this);
  add_check_box(m_side_check_box, tr("Side"), side_action);
  m_table_columns_menu->addAction(side_action);
  auto size_action = new QWidgetAction(this);
  add_check_box(m_size_check_box, tr("Size"), size_action);
  m_table_columns_menu->addAction(size_action);
  auto ref_px_action = new QWidgetAction(this);
  add_check_box(m_ref_px_check_box, tr("Reference Px"), ref_px_action);
  m_table_columns_menu->addAction(ref_px_action);
  auto date_action = new QWidgetAction(this);
  add_check_box(m_date_check_box, tr("Date"), date_action);
  m_table_columns_menu->addAction(date_action);
  auto time_action = new QWidgetAction(this);
  add_check_box(m_time_check_box, tr("Time"), time_action);
  m_table_columns_menu->addAction(time_action);
  auto notional_value_action = new QWidgetAction(this);
  add_check_box(m_notional_value_check_box, tr("Notional Value"),
    notional_value_action);
  m_table_columns_menu->addAction(notional_value_action);
  auto export_table_action = new QAction(tr("Export Table"), this);
  addAction(export_table_action);
  auto export_chart_action = new QAction(tr("Export Chart"), this);
  addAction(export_chart_action);
  auto reset_action = new QAction(tr("Reset All Filters"), this);
  addAction(reset_action);
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

void OrderImbalanceIndicatorContextMenu::add_check_box(CheckBox* check_box,
    const QString& text, QWidgetAction* action) {
  check_box = new CheckBox(text, this);
  check_box->setFixedSize(scale(140, 20));
  check_box->setChecked(true);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  auto metrics = QFontMetrics(font);
  auto spacing = scale_width(108) - metrics.horizontalAdvance(text);
  auto text_style = QString(R"(
    color: black;
    font-family: %4;
    font-size: %1px;
    outline: none;
    padding-right: %3px;
    spacing: %2px;
    text-align: left center;)")
    .arg(font.pixelSize()).arg(spacing).arg(scale_width(8))
    .arg(font.family());
  auto indicator_style = QString(R"(
    background-color: white;
    border: %1px solid #C8C8C8 %2px solid #C8C8C8;
    height: %3px;
    width: %4px;)").arg(scale_height(1))
    .arg(scale_width(1)).arg(scale_height(16)).arg(scale_width(16));
  auto checked_style = QString(R"(
    image: url(:/Icons/check-with-box.svg);)");
  auto hover_style = QString(R"(
    border: %1px solid #4B23A0 %2px solid #4B23A0;)")
    .arg(scale_height(1)).arg(scale_width(1));
  auto focused_style = QString(R"(border-color: #4B23A0;)");
  check_box->set_stylesheet(text_style, indicator_style, checked_style,
    hover_style, focused_style);
  check_box->setLayoutDirection(Qt::RightToLeft);
  action->setDefaultWidget(check_box);
}
