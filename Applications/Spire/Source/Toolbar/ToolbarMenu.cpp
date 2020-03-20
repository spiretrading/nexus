#include "Spire/Toolbar/ToolbarMenu.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Toolbar/MenuIconSizeProxyStyle.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ToolbarMenu::ToolbarMenu(const QString& title, QWidget* parent)
    : QPushButton(title, parent),
      m_items(new QMenu(this)),
      m_menu_icon_style(
        std::make_unique<MenuIconSizeProxyStyle>(scale_width(18))),
      m_empty_style(true) {
  setMenu(m_items);
  m_items->setWindowFlag(Qt::NoDropShadowWindowHint);
  m_drop_shadow = std::make_unique<DropShadow>(false, true, m_items);
  m_items->setStyle(m_menu_icon_style.get());
  connect(m_items, &QMenu::triggered, this, &ToolbarMenu::on_triggered);
  m_empty_item = new QWidgetAction(this);
  m_empty_item->setText(tr("Empty"));
  m_action_to_index[m_empty_item] = 0;
  m_items->addAction(m_empty_item);
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
      height: %3px;
      image: url(":/Icons/arrow-down.svg");
      padding-bottom: %6px;
      padding-right: %2px;
      width: %4px;
    })")
    .arg(scale_height(12)).arg(scale_width(8)).arg(scale_height(4))
    .arg(scale_width(6)).arg(scale_width(8)).arg(scale_height(10)));
  set_empty_menu_stylesheet();
}

void ToolbarMenu::add(const QString& text) {
  auto action = new QWidgetAction(this);
  action->setText(text);
  remove_empty_item();
  if(m_empty_style) {
    m_empty_style = false;
    set_default_menu_stylesheet();
  }
  m_items->addAction(action);
  m_action_to_index[action] = m_action_to_index.size();
}

void ToolbarMenu::add(const QString& text, const QImage& icon) {
  auto action = new QWidgetAction(this);
  action->setText(text);
  action->setIcon(QPixmap::fromImage(icon));
  action->setIconVisibleInMenu(true);
  remove_empty_item();
  if(m_empty_style) {
    m_empty_style = false;
    set_default_menu_stylesheet();
  }
  m_items->addAction(action);
  m_action_to_index[action] = m_action_to_index.size();
}

void ToolbarMenu::remove(int index) {
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
    m_empty_style = true;
  }
}

connection ToolbarMenu::connect_item_selected_signal(
    const ItemSelectedSignal::slot_type& slot) const {
  return m_item_selected_signal.connect(slot);
}

void ToolbarMenu::resizeEvent(QResizeEvent* event) {
  m_items->setFixedWidth(size().width());
}

void ToolbarMenu::remove_empty_item() {
  if(m_action_to_index.find(m_empty_item) != m_action_to_index.end()) {
    m_action_to_index.erase(m_empty_item);
    m_items->removeAction(m_empty_item);
  }
}

void ToolbarMenu::set_empty_menu_stylesheet() {
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

void ToolbarMenu::set_default_menu_stylesheet() {
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
  })").arg(scale_height(12)).arg(scale_width(8)).arg(scale_height(20)));
}

void ToolbarMenu::on_triggered(QAction* action) {
  if(action == m_empty_item) {
    return;
  }
  auto index = m_action_to_index[action];
  m_item_selected_signal(index);
}
