#include "Spire/Ui/ContextMenu.hpp"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QScreen>
#include <QTimer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/SubmenuItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

namespace {
  const auto MAX_WIDTH = 130;
  const auto MARGIN_SIZE = 50;

  auto MARGIN_HEIGHT() {
    static auto height = scale_height(MARGIN_SIZE);
    return height;
  }

  auto MARGIN_WIDTH() {
    static auto width = scale_width(MARGIN_SIZE);
    return width;
  }

  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(EdgeNavigation::CONTAIN).
      set(SelectionMode::NONE);
    return style;
  }
}

ContextMenu::ContextMenu(QWidget& parent)
    : QWidget(&parent),
      m_active_menu_window(nullptr) {
  setAttribute(Qt::WA_Hover);
  setMaximumWidth(scale_width(MAX_WIDTH));
  m_list = std::make_shared<ArrayListModel<MenuItem>>();
  m_list_view = new ListView(m_list,
    std::bind_front(&ContextMenu::build_item, this));
  m_list_view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  update_style(*m_list_view, [&] (auto& style) {
    style = LIST_VIEW_STYLE(style);
  });
  m_list_view->connect_submit_signal(
    std::bind_front(&ContextMenu::on_submit, this));
  m_list_view->installEventFilter(this);
  setFocusProxy(m_list_view);
  enclose(*this, *m_list_view);
  m_window = new OverlayPanel(*this, parent);
  m_window->setWindowFlags(Qt::Popup | (m_window->windowFlags() & ~Qt::Tool));
  m_window->set_closed_on_focus_out(true);
  m_window->set_positioning(OverlayPanel::Positioning::NONE);
  m_window->set_is_draggable(false);
  m_window->setMouseTracking(true);
  m_window->installEventFilter(this);
  m_window->parentWidget()->installEventFilter(this);
  on_window_style();
  m_window_style_connection =
    connect_style_signal(*m_window, [=] { on_window_style(); });
}

void ContextMenu::add_menu(const QString& name, ContextMenu& menu) {
  menu.window()->setParent(this);
  m_list->push(MenuItem(MenuItemType::SUBMENU, name, &menu));
}

void ContextMenu::add_action(const QString& name, const Action& action) {
  m_list->push(MenuItem(MenuItemType::ACTION, name, action));
}

std::shared_ptr<BooleanModel> ContextMenu::add_check_box(const QString& name) {
  auto model = std::make_shared<LocalBooleanModel>();
  add_check_box(name, model);
  return model;
}

void ContextMenu::add_check_box(const QString& name,
    const std::shared_ptr<BooleanModel>& checked) {
  m_list->push(MenuItem(MenuItemType::CHECK, name, checked));
}

connection ContextMenu::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool ContextMenu::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    switch(key_event.key()) {
      case Qt::Key_Right:
        if(watched == m_list_view) {
          handle_right_or_enter_event(event);
        } else if(watched == m_window && !m_active_menu_window) {
          focus_first_item();
        }
        break;
      case Qt::Key_Left:
        if(watched == m_list_view) {
          clear_hover_style();
          QCoreApplication::sendEvent(m_window, event);
        } else if(watched == m_active_menu_window) {
          hide_active_menu();
          clear_hover_style();
        }
        break;
      case Qt::Key_Enter:
      case Qt::Key_Return:
        if(dynamic_cast<const SubmenuItem*>(watched)) {
          handle_right_or_enter_event(event);
        } else if(watched == m_window) {
          focus_first_item();
        }
        break;
    }
  } else if(watched == m_window) {
    if(event->type() == QEvent::Show) {
      show();
      if(!m_window_size) {
        m_window_size = m_window->sizeHint();
        m_window->setFixedSize(*m_window_size);
      }
    } else if(event->type() == QEvent::Close) {
      hide();
    }
  } else if(watched == m_active_menu_window) {
    if(event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::MouseMove) {
      return handle_mouse_event(static_cast<QMouseEvent*>(event));
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool ContextMenu::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_window->show();
    setFocus(Qt::PopupFocusReason);
    if(m_list->get_size() == 0) {
      QTimer::singleShot(10, [=] { hide(); });
    }
  } else if(event->type() == QEvent::HideToParent) {
    m_window->hide();
    hide_active_menu();
    m_list_view->get_current()->set(none);
  } else if(event->type() == QEvent::HoverMove) {
    auto& hover_event = *static_cast<QHoverEvent*>(event);
    for(auto i = 0; i < m_list_view->get_list()->get_size(); ++i) {
      auto item = m_list_view->get_list_item(i);
      if(item->geometry().contains(hover_event.pos())) {
        if(m_list_view->get_current()->get() != i) {
          m_list_view->get_current()->set(i);
          show_submenu(i);
        }
        break;
      }
    }
  } else if(event->type() == QEvent::HoverLeave) {
    m_list_view->get_current()->set(none);
  }
  return QWidget::event(event);
}

QWidget* ContextMenu::build_item(const std::shared_ptr<AnyListModel>& list,
    int index) {
  auto item = std::any_cast<MenuItem&&>(list->get(index));
  if(item.m_type == MenuItemType::ACTION) {
    return make_label(item.m_name);
  } else if(item.m_type == MenuItemType::CHECK) {
    auto check_box =
      new CheckBox(std::get<std::shared_ptr<BooleanModel>>(item.m_data));
    check_box->set_label(item.m_name);
    return check_box;
  }
  auto submenu = std::get<ContextMenu*>(item.m_data);
  auto submenu_item = new SubmenuItem(item.m_name, *submenu);
  submenu_item->installEventFilter(this);
  submenu->connect_submit_signal(
    [=] (const ContextMenu& menu, const QString& label) {
      hide();
      m_submit_signal(menu, label);
    });
  m_submenus[index] = submenu->window();
  return submenu_item;
}

ListItem* ContextMenu::get_current_item() const {
  if(auto current = m_list_view->get_current()->get()) {
    return m_list_view->get_list_item(*current);
  }
  return nullptr;
}

void ContextMenu::clear_hover_style() {
  if(auto item = get_current_item()) {
    auto leave_event = QEvent(QEvent::Leave);
    QCoreApplication::sendEvent(item, &leave_event);
  }
}

void ContextMenu::focus_first_item() {
  if(m_list_view->get_list()->get_size() > 0) {
    m_list_view->get_current()->set(0);
  }
}

void ContextMenu::handle_right_or_enter_event(QEvent* event) {
  if(auto current = m_list_view->get_current()->get()) {
    show_submenu(*current);
    if(m_active_menu_window) {
      QCoreApplication::sendEvent(m_active_menu_window, event);
    }
  }
}

bool ContextMenu::handle_mouse_event(QMouseEvent* event) {
  if(!m_active_menu_window->rect().contains(event->pos())) {
    if(auto item = get_current_item()) {
      if(!item->rect().contains(item->mapFromGlobal(event->globalPos()))) {
        hide_active_menu();
        clear_hover_style();
        m_list_view->get_current()->set(none);
      } else if(event->type() == QEvent::MouseButtonPress ||
          event->type() == QEvent::MouseButtonDblClick) {
        return true;
      }
    }
  }
  return false;
}

void ContextMenu::on_submit(const std::any& submission) {
  auto menu_item = std::any_cast<MenuItem&&>(
    m_list->get(*m_list_view->get_current()->get()));
  if(menu_item.m_type == MenuItemType::ACTION) {
    std::get<Action>(menu_item.m_data)();
    hide();
    m_submit_signal(*this, menu_item.m_name);
  }
}

void ContextMenu::on_window_style() {
  m_window_border_size = QMargins();
  auto& stylist = find_stylist(*m_window);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BorderTopSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_window_border_size.setTop(size);
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_window_border_size.setRight(size);
        });
      },
      [&] (const BorderBottomSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_window_border_size.setBottom(size);
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_window_border_size.setLeft(size);
        });
      });
  }
}

void ContextMenu::position_menu(ListItem* item) {
  auto item_pos = m_list_view->mapToGlobal(item->pos());
  auto menu_geomerty = m_window->geometry();
  auto screen_geometry = screen()->availableGeometry();
  auto menu_margins = m_window->layout()->contentsMargins();
  auto active_menu_margins = m_active_menu_window->layout()->contentsMargins();
  auto x = [&] {
    auto right = menu_geomerty.right() - menu_margins.right();
    if(screen_geometry.right() - right >
        m_active_menu_window->width() + MARGIN_WIDTH()) {
      return right - menu_margins.left();
    } else {
      return menu_geomerty.x() + menu_margins.left() -
        m_active_menu_window->width() + active_menu_margins.right();
    }
  }();
  auto y = [&] {
    if(screen_geometry.bottom() - item_pos.y() + m_window_border_size.top() >
        m_active_menu_window->height() + MARGIN_HEIGHT()) {
      return item_pos.y() - menu_margins.top() - m_window_border_size.top();
    } else {
      return screen_geometry.bottom() - m_active_menu_window->height() -
        MARGIN_HEIGHT() + active_menu_margins.bottom();
    }
  }();
  m_active_menu_window->move(x, y);
}

void ContextMenu::hide_active_menu() {
  if(m_active_menu_window) {
    m_active_menu_window->hide();
    m_active_menu_window->removeEventFilter(this);
    m_active_menu_window = nullptr;
  }
}

void ContextMenu::show_submenu(int index) {
  auto menu_item = std::any_cast<MenuItem&&>(m_list->get(index));
  if(menu_item.m_type == MenuItemType::SUBMENU) {
    auto menu_window = m_submenus[index];
    if(m_active_menu_window == menu_window) {
      return;
    }
    hide_active_menu();
    m_active_menu_window = menu_window;
    m_active_menu_window->show();
    position_menu(m_list_view->get_list_item(index));
    m_active_menu_window->installEventFilter(this);
  }
}
