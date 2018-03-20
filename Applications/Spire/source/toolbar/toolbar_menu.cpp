#include "spire/toolbar/toolbar_menu.hpp"
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPoint>
#include <QStyle>
#include <QtSvg/QSvgRenderer>
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

toolbar_menu::toolbar_menu(const QString& title, QWidget* parent)
    : QPushButton(title, parent),
      m_items(new QMenu(this)),
      m_default_style(true) {
  setMenu(m_items);
  setIconSize(QSize(500, 500));
  m_items->installEventFilter(this);
  connect(m_items, &QMenu::triggered, this, &toolbar_menu::on_triggered);
  m_empty_item = new QWidgetAction(this);
  m_empty_item->setText(tr("Empty"));
  m_action_to_index[m_empty_item] = 0;
  m_items->addAction(m_empty_item);
  set_stylesheet(0);
}

void toolbar_menu::add(const QString& text) {
  auto action = new QWidgetAction(this);
  action->setText(text);
  remove_empty_item();
  if(m_default_style) {
    m_default_style = false;
    set_default_menu_stylesheet(scale_width(8));
  }
  m_items->addAction(action);
  auto size = m_action_to_index.size();
  m_action_to_index[action] = size;
}

void toolbar_menu::add(const QString& text, const QString& icon_path) {
  if(m_filepath_to_pixmap.find(icon_path.toStdString()) ==
      m_filepath_to_pixmap.end()) {
    auto renderer = new QSvgRenderer(icon_path, this);
    auto icon = QImage(scale_width(10), scale_height(10),
      QImage::Format_ARGB32);
    icon.fill(QColor(0, 0, 0, 0));
    QPainter icon_painter(&icon);
    renderer->render(&icon_painter);
    icon_painter.end();
    auto background = QImage(scale_width(26), scale_height(20),
      QImage::Format_ARGB32);
    background.fill(QColor(0, 0, 0, 0));
    QPainter background_painter(&background);
    background_painter.drawImage(
      QPoint(scale_width(8), scale_height(5)), icon);
    background_painter.end();
    m_filepath_to_pixmap[icon_path.toStdString()] =
      QPixmap::fromImage(background);
  }
  auto action = new QWidgetAction(this);
  action->setText(text);
  action->setIcon(m_filepath_to_pixmap[icon_path.toStdString()]);
  action->setIconVisibleInMenu(true);
  remove_empty_item();
  if(m_default_style) {
    m_default_style = false;
    set_default_menu_stylesheet(scale_width(26));
  }
  m_items->addAction(action);
  auto size = m_action_to_index.size();
  m_action_to_index[action] = size;
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
  if(m_action_to_index.size() == 0) {
    m_action_to_index[m_empty_item] = 0;
    m_items->addAction(m_empty_item);
    set_empty_menu_stylesheet();
    m_default_style = true;
  }
}

connection toolbar_menu::connect_item_selected_signal(
    const item_selected_signal::slot_type& slot) const {
  return m_item_selected_signal.connect(slot);
}

void toolbar_menu::leaveEvent(QEvent* event) {
  if(hasFocus()) {
    
  } else {

  }
}

void toolbar_menu::resizeEvent(QResizeEvent* event) {
  m_items->setFixedWidth(size().width());
}

void toolbar_menu::remove_empty_item() {
  if(m_action_to_index.find(m_empty_item) != m_action_to_index.end()) {
    m_action_to_index.erase(m_empty_item);
    m_items->removeAction(m_empty_item);
  }
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

    QPushButton:focus, QPushButton:hover {
      border: 1px solid #4B23A0;
      outline: none;
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

  set_empty_menu_stylesheet();
}

void toolbar_menu::set_empty_menu_stylesheet() {
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
    color: #8C8C8C;
    font-style: italic;
    height: %2px;
    padding-left: %3px;
  })").arg(scale_height(12)).arg(scale_height(20)).arg(scale_width(8)));
}

void toolbar_menu::set_default_menu_stylesheet(int padding_left) {
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
  if(action == m_empty_item) {
    return;
  }
  auto index = m_action_to_index[action];
  m_item_selected_signal(index);
}
