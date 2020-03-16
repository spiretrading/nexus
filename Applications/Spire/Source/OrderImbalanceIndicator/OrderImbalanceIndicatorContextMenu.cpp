#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorContextMenu.hpp"
#include <QMenu>
#include "Spire/Spire/Dimensions.hpp"
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
  auto table_columns_menu = new QMenu(tr("Table Columns"), this);
  addMenu(table_columns_menu);
  auto export_table_action = new QAction(tr("Export Table"), this);
  addAction(export_table_action);
  auto export_chart_action = new QAction(tr("Export Chart"), this);
  addAction(export_chart_action);
  auto reset_all_filters_action = new QAction(tr("Reset All Filters"), this);
  addAction(reset_all_filters_action);
}
