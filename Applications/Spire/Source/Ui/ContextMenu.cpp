#include "Spire/Ui/ContextMenu.hpp"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QScreen>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Icon.hpp"
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
  const auto MIN_WIDTH = 130;

  auto MENU_SHOW_DELAY() {
    #ifdef Q_OS_WIN
      auto delay = UINT(400);
      SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &delay, 0);
      return static_cast<int>(delay);
    #else
      return 400;
    #endif
  }

  auto OVERLAP_WIDTH() {
    static auto size = scale_width(5);
    return size;
  }

  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(EdgeNavigation::CONTAIN);
    style.get(
      Any() > (is_a<ListItem>() && Hover() && !Current() && !Selected())).
      set(BackgroundColor(QColor(0xFFFFFF)));
    style.get(Any() > (is_a<ListItem>() && Disabled()) > is_a<TextBox>()).
      set(TextColor(QColor(0xC8C8C8)));
    return style;
  }

  auto make_icon_item(const QString& name, QImage icon) {
    auto item = new QWidget();
    auto layout = make_hbox_layout(item);
    layout->addWidget(new Icon(std::move(icon)), 0, Qt::AlignVCenter);
    layout->addSpacing(scale_width(8));
    layout->addWidget(make_label(name));
    return item;
  }

  QWidget* default_item_view_builder(ContextMenu::MenuItemType type,
      const QString& name, const ContextMenu::Data& data) {
    if(type == ContextMenu::MenuItemType::ACTION) {
      return make_label(name);
    } else if(type == ContextMenu::MenuItemType::DISABLED_ACTION) {
      auto label = make_label(name);
      label->setSizePolicy(
        QSizePolicy::Expanding, label->sizePolicy().verticalPolicy());
      update_style(*label, [] (auto& style) {
        auto font = QFont("Roboto");
        font.setWeight(QFont::Normal);
        font.setPixelSize(scale_width(12));
        font.setItalic(true);
        style.get(Any()).
          set(Font(font)).
          set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
          set(TextColor(QColor(Qt::black)));
      });
      return label;
    } else if(type == ContextMenu::MenuItemType::CHECK) {
      auto check_box =
        new CheckBox(std::get<std::shared_ptr<BooleanModel>>(data));
      check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      check_box->setLayoutDirection(Qt::RightToLeft);
      check_box->set_label(name);
      return check_box;
    } else if(type == ContextMenu::MenuItemType::SEPARATOR) {
      auto separator = new Box();
      separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      separator->setFixedHeight(scale_height(1));
      update_style(*separator, [] (auto& style) {
        style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
      });
      return separator;
    }
    return nullptr;
  }

  auto is_point_in_triangle(const QPoint& p, const QPoint& a, const QPoint& b,
      const QPoint& c) {
    auto sign = [] (const QPoint& p1, const QPoint& p2, const QPoint& p3) {
      return (p1.x() - p3.x()) * (p2.y() - p3.y()) -
        (p2.x() - p3.x()) * (p1.y() - p3.y());
    };
    auto b1 = sign(p, a, b) < 0;
    auto b2 = sign(p, b, c) < 0;
    auto b3 = sign(p, c, a) < 0;
    return (b1 == b2) && (b2 == b3);
  }
}

ContextMenu::ContextMenu(QWidget& parent)
  : ContextMenu(parent, &default_item_view_builder) {}

ContextMenu::ContextMenu(QWidget& parent, ItemViewBuilder item_view_builder)
    : QWidget(&parent),
      m_item_view_builder(std::move(item_view_builder)),
      m_next_id(0),
      m_visible_submenu(nullptr),
      m_pending_item_index(-1),
      m_last_show_items(0),
      m_block_move(0),
      m_mouse_observer(*this),
      m_defer_hide_timer(this) {
  setAttribute(Qt::WA_Hover);
  setMinimumWidth(scale_width(MIN_WIDTH));
  m_list = std::make_shared<ArrayListModel<MenuItem>>();
  m_list_view = new ListView(
    m_list, std::make_shared<ListEmptySelectionModel>(),
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
  m_window->setUpdatesEnabled(false);
  m_window->setWindowFlags(Qt::Popup | (m_window->windowFlags() & ~Qt::Tool));
  m_window->set_closed_on_focus_out(true);
  m_window->set_positioning(OverlayPanel::Positioning::NONE);
  m_window->set_is_draggable(false);
  m_window->setMouseTracking(true);
  m_window->installEventFilter(this);
  m_window->parentWidget()->installEventFilter(this);
  on_window_style();
  m_window_style_connection = connect_style_signal(
    *m_window, std::bind_front(&ContextMenu::on_window_style, this));
  m_list->connect_operation_signal(
    std::bind_front(&ContextMenu::on_list_operation, this));
  m_mouse_observer.connect_move_signal(
    std::bind_front(&ContextMenu::on_mouse_move, this));
  m_defer_hide_timer.setSingleShot(true);
  connect(&m_defer_hide_timer, &QTimer::timeout,
    std::bind_front(&ContextMenu::on_defer_hide_timeout, this));
}

void ContextMenu::add_menu(const QString& name, ContextMenu& menu) {
  menu.window()->setParent(this);
  m_list->push(MenuItem(++m_next_id, MenuItemType::SUBMENU, name, &menu));
}

void ContextMenu::add_action(const QString& name, const Action& action) {
  m_list->push(MenuItem(++m_next_id, MenuItemType::ACTION, name, action));
}

void ContextMenu::add_action(
    const QString& name, const Action& action, QWidget* view) {
  auto id = ++m_next_id;
  m_custom_views.insert(std::pair(id, view));
  m_list->push(MenuItem(id, MenuItemType::ACTION, name, action));
}

void ContextMenu::add_disabled_action(const QString& name) {
  m_list->push(MenuItem(++m_next_id, MenuItemType::DISABLED_ACTION, name));
  auto item = m_list_view->get_list_item(m_list->get_size() - 1);
  item->setDisabled(true);
}

std::shared_ptr<BooleanModel> ContextMenu::add_check_box(const QString& name) {
  auto model = std::make_shared<LocalBooleanModel>();
  add_check_box(name, model);
  return model;
}

void ContextMenu::add_check_box(const QString& name,
    const std::shared_ptr<BooleanModel>& checked) {
  m_list->push(MenuItem(++m_next_id, MenuItemType::CHECK, name, checked));
}

void ContextMenu::add_separator() {
  m_list->push(MenuItem(++m_next_id, MenuItemType::SEPARATOR));
}

void ContextMenu::reset() {
  clear(*m_list);
}

int ContextMenu::get_count() const {
  return m_list->get_size();
}

QWidget* ContextMenu::get_menu_item(int index) {
  return m_list_view->get_list_item(index);
}

connection ContextMenu::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_list_view->get_current()->connect_update_signal(slot);
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
        } else if(watched == m_window && !m_visible_submenu) {
          focus_first_item();
        }
        break;
      case Qt::Key_Left:
        if(watched == m_list_view) {
          clear_hover_style();
          QCoreApplication::sendEvent(m_window, event);
        } else if(watched == m_visible_submenu) {
          hide_submenu();
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
  } else if(watched == m_visible_submenu) {
    if(event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::MouseMove) {
      return handle_mouse_event(static_cast<QMouseEvent*>(event));
    } else if(event->type() == QEvent::Move ||
        event->type() == QEvent::Resize) {
      position_submenu();
    }
  } else if(m_visible_submenu && watched ==
      &m_visible_submenu->get_body() && event->type() == QEvent::Resize) {
    position_submenu();
  }
  return QWidget::eventFilter(watched, event);
}

bool ContextMenu::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    if(m_last_show_items != m_list_view->get_list()->get_size()) {
      m_window->setWindowOpacity(0.0);
    }
    m_window->show();
    m_last_show_items = m_list_view->get_list()->get_size();
    setFocus(Qt::PopupFocusReason);
    if(m_list->get_size() == 0) {
      QTimer::singleShot(10, [=] { m_window->hide(); });
    }
  } else if(event->type() == QEvent::HideToParent) {
    m_window->hide();
    hide_submenu();
    m_list_view->get_current()->set(none);
    m_list_view->setFocusProxy(nullptr);
  } else if(event->type() == QEvent::HoverLeave) {
    m_list_view->get_current()->set(none);
    m_list_view->setFocusProxy(nullptr);
    m_list_view->setFocus();
  }
  return QWidget::event(event);
}

void ContextMenu::hideEvent(QHideEvent* event) {
  m_window->hide();
  hide_submenu();
  m_list_view->get_current()->set(none);
  m_list_view->setFocusProxy(nullptr);
}

void ContextMenu::resizeEvent(QResizeEvent* event) {
  m_window->setWindowOpacity(1.0);
  if(!m_window->updatesEnabled() &&
      event->size().height() >= scale_height(10)) {
    m_window->setUpdatesEnabled(true);
  }
}

QWidget* ContextMenu::build_item(
    const std::shared_ptr<AnyListModel>& list, int index) {
  auto item = std::any_cast<MenuItem>(list->get(index));
  auto custom_view = m_custom_views.find(item.m_id);
  if(custom_view != m_custom_views.end()) {
    return custom_view->second;
  }
  if(item.m_type == MenuItemType::SUBMENU) {
    auto submenu = std::get<ContextMenu*>(item.m_data);
    auto submenu_item = new SubmenuItem(item.m_name, *submenu);
    submenu_item->installEventFilter(this);
    submenu->connect_submit_signal(
      [=] (const ContextMenu& menu, const QString& label) {
        m_window->hide();
        m_submit_signal(menu, label);
      });
    m_submenus[index] = static_cast<OverlayPanel*>(submenu->window());
    return submenu_item;
  }
  return m_item_view_builder(item.m_type, item.m_name, item.m_data);
}

ListItem* ContextMenu::get_current_item() const {
  if(auto current = m_list_view->get_current()->get()) {
    return m_list_view->get_list_item(*current);
  }
  return nullptr;
}

int ContextMenu::find_hovered_item(const QPoint& global_pos) const {
  for(auto i = 0; i < m_list_view->get_list()->get_size(); ++i) {
    auto item = m_list_view->get_list_item(i);
    if(item->rect().contains(item->mapFromGlobal(global_pos))) {
      return i;
    }
  }
  return -1;
}

bool ContextMenu::is_mouse_approaching_submenu() const {
  if(!m_visible_submenu || m_mouse_history.size() < 2) {
    return false;
  }
  auto& body = static_cast<ContextMenu&>(m_visible_submenu->get_body());
  auto body_rect = body.rect();
  auto upper_left = body.mapToGlobal(body_rect.topLeft());
  auto lower_left = body.mapToGlobal(body_rect.bottomLeft());
  return is_point_in_triangle(m_mouse_history.back(), m_mouse_history.front(),
    upper_left, lower_left);
}

void ContextMenu::apply_hover_style() {
  if(auto item = get_current_item()) {
    auto enter_event = QEvent(QEvent::Enter);
    QCoreApplication::sendEvent(item, &enter_event);
  }
}

void ContextMenu::clear_hover_style() {
  if(auto item = get_current_item()) {
    auto leave_event = QEvent(QEvent::Leave);
    QCoreApplication::sendEvent(item, &leave_event);
  }
}

void ContextMenu::focus_first_item() {
  QTimer::singleShot(0, m_list_view, [=] {
    if(m_list_view->get_list()->get_size() > 0) {
      m_list_view->get_current()->set(0);
      m_list_view->get_list_item(0)->setFocus();
    }
  });
}

void ContextMenu::handle_right_or_enter_event(QEvent* event) {
  if(auto current = m_list_view->get_current()->get()) {
    show_submenu(*current);
    if(m_visible_submenu) {
      QCoreApplication::sendEvent(m_visible_submenu, event);
    }
  }
}

bool ContextMenu::handle_mouse_event(QMouseEvent* event) {
  if(!m_visible_submenu->rect().contains(event->pos())) {
    if(auto item = get_current_item()) {
      if(!item->rect().contains(item->mapFromGlobal(event->globalPos()))) {
        clear_hover_style();
        if(event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonDblClick) {
          forward_mouse_click(*this, *event);
        }
      } else if(event->type() == QEvent::MouseButtonPress ||
          event->type() == QEvent::MouseButtonDblClick) {
        return true;
      }
    }
  }
  return false;
}

void ContextMenu::forward_mouse_click(QWidget& target,
    const QMouseEvent& mouse_event) {
  auto press_event = QMouseEvent(mouse_event.type(),
    target.mapFromGlobal(mouse_event.globalPos()),
    mouse_event.windowPos(), mouse_event.screenPos(),
    mouse_event.button(), mouse_event.buttons(), mouse_event.modifiers(),
    mouse_event.source());
  QCoreApplication::sendEvent(&target, &press_event);
  auto release_event = QMouseEvent(QEvent::MouseButtonRelease,
    target.mapFromGlobal(mouse_event.globalPos()),
    mouse_event.windowPos(), mouse_event.screenPos(),
    mouse_event.button(), mouse_event.buttons(), mouse_event.modifiers(),
    mouse_event.source());
  QCoreApplication::sendEvent(&target, &release_event);
}

void ContextMenu::position_submenu() {
  if(!m_visible_submenu || m_block_move != 0) {
    return;
  }
  auto screen_geometry = screen()->availableGeometry();
  auto body_size = m_visible_submenu->get_body().size();
  auto menu_size = m_visible_submenu->size();
  auto active_menu_margins = m_visible_submenu->layout()->contentsMargins();
  auto candidate_right = m_active_item_geometry.right() -
    OVERLAP_WIDTH() - active_menu_margins.left() + m_window_border_size.right();
  auto candidate_top = m_active_item_geometry.top() -
    active_menu_margins.top() - m_window_border_size.top();
  auto candidate_bottom = m_active_item_geometry.bottom() -
    body_size.height() - active_menu_margins.top() - m_window_border_size.top();
  auto candidate_left = m_active_item_geometry.left() + OVERLAP_WIDTH() -
    body_size.width() - active_menu_margins.right() -
    m_window_border_size.left();
  auto candidate_geometry =
    QRect(QPoint(candidate_right, candidate_top), menu_size);
  if(!screen_geometry.contains(candidate_geometry)) {
    candidate_geometry = QRect();
  }
  if(candidate_geometry.isNull()) {
    candidate_geometry =
      QRect(QPoint(candidate_right, candidate_bottom), menu_size);
    if(!screen_geometry.contains(candidate_geometry)) {
      candidate_geometry = QRect();
    }
  }
  if(candidate_geometry.isNull()) {
    candidate_geometry =
      QRect(QPoint(candidate_left, candidate_top), menu_size);
    if(!screen_geometry.contains(candidate_geometry)) {
      candidate_geometry = QRect();
    }
  }
  if(candidate_geometry.isNull()) {
    candidate_geometry =
      QRect(QPoint(candidate_left, candidate_bottom), menu_size);
  }
  if(m_visible_submenu->pos() != candidate_geometry.topLeft()) {
    ++m_block_move;
    m_visible_submenu->move(candidate_geometry.topLeft());
    --m_block_move;
  }
}

void ContextMenu::position_submenu(ListItem& item) {
  m_active_item_geometry =
    QRect(item.mapToGlobal(QPoint(0, 0)), item.geometry().size());
  position_submenu();
}

bool ContextMenu::is_submenu_hovered() const {
  if(!m_visible_submenu) {
    return false;
  }
  auto& body = static_cast<ContextMenu&>(m_visible_submenu->get_body());
  if(body.rect().contains(body.mapFromGlobal(QCursor::pos()))) {
    return true; 
  }
  return body.is_submenu_hovered();
}

void ContextMenu::hide_submenu() {
  if(m_visible_submenu) {
    auto& body = static_cast<ContextMenu&>(m_visible_submenu->get_body());
    body.m_list_view->get_current()->set(none);
    body.m_list_view->setFocusProxy(nullptr);
    body.removeEventFilter(this);
    m_visible_submenu->hide();
    m_visible_submenu->removeEventFilter(this);
    m_visible_submenu = nullptr;
    m_mouse_history.clear();
  }
}

void ContextMenu::cancel_defer_hide_submenu() {
  if(m_defer_hide_timer.isActive()) {
    m_defer_hide_timer.stop();
  }
}

void ContextMenu::defer_hide_submenu() {
  if(m_visible_submenu) {
    m_defer_hide_timer.start(MENU_SHOW_DELAY());
  }
}

void ContextMenu::show_submenu(int index) {
  auto& menu_item = m_list->get(index);
  if(menu_item.m_type == MenuItemType::SUBMENU) {
    auto menu_window = m_submenus[index];
    m_visible_submenu = menu_window;
    m_visible_submenu->installEventFilter(this);
    m_visible_submenu->get_body().installEventFilter(this);
    position_submenu(*m_list_view->get_list_item(index));
    auto& active_menu =
      static_cast<ContextMenu&>(m_visible_submenu->get_body());
    if(active_menu.m_list->get_size() != 0) {
      m_visible_submenu->show();
      m_mouse_history.push_back(QCursor::pos());
    }
  }
}

void ContextMenu::on_defer_hide_timeout() {
  hide_submenu();
  if(m_pending_item_index != -1) {
    m_list_view->get_current()->set(m_pending_item_index);
    apply_hover_style();
    show_submenu(m_pending_item_index);
    m_pending_item_index = -1;
  }
}

void ContextMenu::on_mouse_move(QWidget& target, QMouseEvent& event) {
  if(is_submenu_hovered()) {
    cancel_defer_hide_submenu();
    return;
  }
  auto global_pos = event.globalPos();
  int hovered_index = find_hovered_item(global_pos);
  if(hovered_index == -1) {
    return;
  }
  if(m_visible_submenu) {
    m_mouse_history.push_back(global_pos);
    static const auto MAX_HISTORY_SIZE = 5;
    if(static_cast<int>(m_mouse_history.size()) > MAX_HISTORY_SIZE) {
      m_mouse_history.pop_front();
    }
    if(m_list_view->get_current()->get() != hovered_index) {
      if(is_mouse_approaching_submenu()) {
        m_pending_item_index = hovered_index;
        defer_hide_submenu();
      } else {
        m_pending_item_index = -1;
        cancel_defer_hide_submenu();
        if(!is_submenu_hovered()) {
          hide_submenu();
        }
      }
    } else {
      apply_hover_style();
    }
  } else if(m_list_view->get_current()->get() != hovered_index) {
    m_list_view->get_current()->set(hovered_index);
    apply_hover_style();
    show_submenu(hovered_index);
  }
}

void ContextMenu::on_list_operation(
    const ListModel<MenuItem>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<MenuItem>::AddOperation& operation) {
      if(auto item = m_list_view->get_list_item(operation.m_index)) {
        switch(m_list->get(operation.m_index).m_type) {
          case MenuItemType::SEPARATOR:
            item->setEnabled(false);
            update_style(*item, [] (auto& style) {
              style.get(Any()).
                set(border_size(0)).
                set(horizontal_padding(0));
            });
            break;
          case MenuItemType::CHECK:
            m_check_item_press_observers.emplace(
              std::piecewise_construct,
              std::forward_as_tuple(operation.m_index),
              std::forward_as_tuple(*item));
            m_check_item_press_observers.at(operation.m_index).
              connect_press_end_signal([=] (auto reason) {
                auto& check_box = static_cast<CheckBox&>(item->get_body());
                if(reason == PressObserver::Reason::MOUSE &&
                    !check_box.rect().contains(check_box.mapFromGlobal(
                      QCursor::pos()))) {
                  check_box.get_current()->set(!check_box.get_current()->get());
                  check_box.setFocus();
                }
              });
            update_style(*item, [] (auto& style) {
              style.get(Any()).set(vertical_padding(scale_height(4)));
              style.get(Hover() > Body() > is_a<Box>()).
                set(border_color(QColor(0x4B23A0)));
            });
            break;
          default:
            break;
        }
      }
    },
    [&] (const ListModel<MenuItem>::PreRemoveOperation& operation) {
      if(m_list->get(operation.m_index).m_type == MenuItemType::CHECK) {
        m_check_item_press_observers.erase(operation.m_index);
      }
    });
}

void ContextMenu::on_submit(const std::any& submission) {
  auto& menu_item = m_list->get(*m_list_view->get_current()->get());
  if(menu_item.m_type == MenuItemType::ACTION) {
    std::get<Action>(menu_item.m_data)();
    m_window->hide();
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

void Spire::show_under_cursor(ContextMenu& menu) {
  auto cursor_position = QCursor::pos();
  auto margins = menu.window()->layout()->contentsMargins();
  menu.window()->move(cursor_position - QPoint(margins.left(), margins.top()));
  menu.show();
}

void Spire::add_action(ContextMenu& menu, const QString& name, QImage icon,
    const ContextMenu::Action& action) {
  menu.add_action(name, action, make_icon_item(name, std::move(icon)));
}
