#include "spire/toolbar/toolbar_menu.hpp"
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QtSvg/QSvgRenderer>
#include <QWidgetAction>
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

toolbar_menu::toolbar_menu(const QString& title, QWidget* parent)
    : QPushButton(title, parent),
      m_items(new QMenu(this)),
      m_default_style(true) {
  setMenu(m_items);
  connect(m_items, &QMenu::triggered, this, &toolbar_menu::on_triggered);
  set_stylesheet(scale_width(8));
}

void toolbar_menu::add(const QString& text) {
  auto action = new QWidgetAction(this);
  action->setText(text);
  m_items->addAction(action);
  auto size = m_action_to_index.size();
  m_action_to_index[action] = size;
}

void toolbar_menu::add(const QString& text, const QString& icon) {
  auto action = new QWidgetAction(this);
  action->setText(text);
  auto renderer = new QSvgRenderer(icon, this);
  auto file = QImage(scale_width(15), scale_height(15), QImage::Format_ARGB32);
  QPainter painter(&file);
  renderer->render(&painter);
  auto background = QImage(scale_width(26), scale_height(20),
    QImage::Format_ARGB32);
  background.fill(QColor(0, 0, 0, 0));
  QPainter painter2(&background);
  painter2.drawImage(QPoint(scale_width(12), scale_width(2)), file);
  painter2.end();
  action->setIcon(QPixmap::fromImage(background));
  action->setIconVisibleInMenu(true);
  m_items->addAction(action);
  auto size = m_action_to_index.size();
  m_action_to_index[action] = size;
  if(m_default_style) {
    m_default_style = false;
    set_stylesheet(scale_width(26));
  }
}

void toolbar_menu::remove(int index) {
  auto action = [&] () -> QAction* {
    for(auto& item : m_action_to_index) {
      if(item.second == index) {
        return item.first;
      }
    }
    return nullptr;
  }();
  m_items->removeAction(action);
  m_action_to_index.erase(action);
  delete action;
  for(auto& item : m_action_to_index) {
    if(item.second > index) {
      --item.second;
    }
  }
}

connection toolbar_menu::connect_item_selected_signal(
    const item_selected_signal::slot_type& slot) const {
  return m_item_selected_signal.connect(slot);
}

void toolbar_menu::resizeEvent(QResizeEvent* event) {
  m_items->setFixedWidth(size().width());
}

void toolbar_menu::set_stylesheet(int padding_left) {
  setStyleSheet(QString(R"(
    QPushButton {
      background-color: white;
      border: 1px solid #C8C8C8;
      font-family: Roboto;
      font-size: %1px;
      padding-left: %5px;
      text-align: left;
    }

    QPushButton:hover {
      border: 1px solid #4B23A0;
    }

    QPushButton::menu-indicator {
      image: url(":/icons/arrow-down.svg");
      height: %3px;
      padding-bottom: %6px;
      padding-right: %2px;
      width: %4px;
    })")
    .arg(scale_height(12)).arg(scale_width(8)).arg(scale_height(4))
    .arg(scale_width(6)).arg(scale_width(8)).arg(scale_height(10)));

  m_items->setStyleSheet(QString(R"(
    QMenu {
      background-color: white;
      border-left: 1px solid #A0A0A0;
      border-right: 1px solid #A0A0A0;
      border-bottom: 1px solid #A0A0A0;
      font-family: Roboto;
      font-size: %1px;
      padding: 0px;
    }

    QMenu::item {
      height: %3px;
      padding-left: %2px;
    }

    QMenu::item:selected {
      background-color: #F2F2FF;
    })").arg(scale_height(12)).arg(padding_left).arg(scale_height(20)));
}

void toolbar_menu::on_triggered(QAction* action) {
  auto index = m_action_to_index[action];
  m_item_selected_signal(index);
}
