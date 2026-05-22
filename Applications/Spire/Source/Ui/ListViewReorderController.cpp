#include "Spire/Ui/ListViewReorderController.hpp"
#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  QWidget* make_item_preview(ListItem& item, QWidget* parent = nullptr) {
    auto pixmap = QPixmap(item.size());
    pixmap.fill(QColor(0xFFFFFF));
    auto& body = item.get_body();
    auto body_pos = body.mapTo(&item, QPoint(0, 0));
    body.render(&pixmap, body_pos, QRegion(), QWidget::DrawChildren);
    auto label = new QLabel();
    label->setPixmap(pixmap);
    auto preview = new Box(label, parent);
    preview->setAttribute(Qt::WA_TransparentForMouseEvents);
    update_style(*preview, [] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0x4B23A0)));
    });
    auto effect = new QGraphicsOpacityEffect(preview);
    effect->setOpacity(0.8);
    preview->setGraphicsEffect(effect);
    preview->setFixedSize(item.size());
    return preview;
  }
}

struct ListViewReorderController::DropItem : QWidget {
  Qt::Orientation m_direction;
  QBoxLayout* m_layout;

  explicit DropItem(Qt::Orientation direction, QWidget* parent = nullptr)
      : QWidget(parent),
        m_direction(direction) {
    auto indicator = new Box(nullptr);
    indicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*indicator, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0x4B23A0)));
    });
    if(direction == Qt::Vertical) {
      indicator->setFixedHeight(scale_height(2));
    } else {
      indicator->setFixedWidth(scale_width(2));
    }
    auto body = new QWidget();
    auto layout_direction = QBoxLayout::LeftToRight;
    if(direction == Qt::Vertical) {
      layout_direction = QBoxLayout::TopToBottom;
    }
    m_layout = new QBoxLayout(layout_direction, body);
    m_layout->setContentsMargins({});
    m_layout->setSpacing(0);
    m_layout->addWidget(indicator);
    m_layout->addStretch(1);
    auto box = new Box(body);
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
    });
    enclose(*this, *box);
  }

  void set_indicator_leading(bool is_leading) {
    if(m_direction == Qt::Vertical) {
      if(is_leading) {
        m_layout->setDirection(QBoxLayout::TopToBottom);
      } else {
        m_layout->setDirection(QBoxLayout::BottomToTop);
      }
    } else if(is_leading) {
      m_layout->setDirection(QBoxLayout::LeftToRight);
    } else {
      m_layout->setDirection(QBoxLayout::RightToLeft);
    }
  }
};

ListViewReorderController::ListViewReorderController(ListView& list_view)
    : QObject(&list_view),
      m_list_view(&list_view),
      m_drop_item(nullptr),
      m_item_preview(nullptr),
      m_drag_item(nullptr),
      m_direction(Qt::Vertical),
      m_current_index(-1),
      m_drag_index(-1),
      m_press_index(-1) {
  m_list_view->setMouseTracking(true);
  m_list_view->installEventFilter(this);
  on_style();
  m_style_connection = connect_style_signal(*m_list_view,
    std::bind_front(&ListViewReorderController::on_style, this));
}

bool ListViewReorderController::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_list_view) {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.modifiers() & Qt::AltModifier && !m_drag_item) {
        auto current = m_list_view->get_current()->get();
        if(current) {
          auto source = *current;
          auto size = m_list_view->get_list()->get_size();
          auto destination = -1;
          switch(key_event.key()) {
            case Qt::Key_Up:
              if(m_direction == Qt::Vertical) {
                destination = source - 1;
              } else {
                destination = find_cross_index(source, -1);
              }
              break;
            case Qt::Key_Down:
              if(m_direction == Qt::Vertical) {
                destination = source + 1;
              } else {
                destination = find_cross_index(source, 1);
              }
              break;
            case Qt::Key_Left:
              if(m_direction == Qt::Horizontal) {
                destination = source - 1;
              } else {
                destination = find_cross_index(source, -1);
              }
              break;
            case Qt::Key_Right:
              if(m_direction == Qt::Horizontal) {
                destination = source + 1;
              } else {
                destination = find_cross_index(source, 1);
              }
              break;
            case Qt::Key_Home:
              destination = 0;
              break;
            case Qt::Key_End:
              destination = size - 1;
              break;
            default:
              break;
          }
          if(destination >= 0 && destination < size && destination != source) {
            keyboard_move(source, destination);
            return true;
          }
        }
      }
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::LeftButton) {
        if(auto index = find_item_index(mouse_event.globalPos()); index >= 0) {
          m_press_position = mouse_event.globalPos();
          m_press_index = index;
        }
      }
    } else if(event->type() == QEvent::MouseMove) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(m_drag_item) {
        drag_move(mouse_event);
      } else if(m_press_index >= 0) {
        if((mouse_event.globalPos() - m_press_position).manhattanLength() >=
            QApplication::startDragDistance()) {
          start_drag(m_press_index, mouse_event);
        }
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      if(m_drag_item) {
        stop_drag();
      }
      m_press_index = -1;
    }
  } else if(watched == m_drag_item && event->type() == QEvent::Move) {
    update_drop_item();
  }
  return QObject::eventFilter(watched, event);
}

int ListViewReorderController::find_cross_index(
    int current, int direction) const {
  auto current_item = m_list_view->get_list_item(current);
  if(!current_item) {
    return -1;
  }
  auto current_rect =
    QRect(current_item->mapToGlobal(QPoint(0, 0)), current_item->size());
  auto navigation_box = current_rect;
  auto candidate = -1;
  auto i = current + direction;
  while(i >= 0 && i < m_list_view->get_list()->get_size()) {
    if(auto item = m_list_view->get_list_item(i)) {
      auto item_rect = QRect(item->mapToGlobal(QPoint(0, 0)), item->size());
      if(candidate == -1) {
        if(m_direction == Qt::Horizontal) {
          navigation_box.moveTop(item_rect.y());
        } else {
          navigation_box.moveLeft(item_rect.x());
        }
      }
      if(((m_direction == Qt::Horizontal &&
            direction * current_rect.y() < direction * navigation_box.y()) ||
          (m_direction == Qt::Vertical &&
            direction * current_rect.x() < direction * navigation_box.x())) &&
          navigation_box.intersects(item_rect)) {
        candidate = i;
        if(direction == 1) {
          break;
        }
      } else if(candidate != -1) {
        break;
      }
    }
    i += direction;
  }
  return candidate;
}

int ListViewReorderController::find_item_index(const QPoint& position) const {
  for(auto i = 0; i < m_list_view->get_list()->get_size(); ++i) {
    auto item = m_list_view->get_list_item(i);
    if(item->rect().contains(item->mapFromGlobal(position))) {
      return i;
    }
  }
  return -1;
}

void ListViewReorderController::start_drag(int index,
    const QMouseEvent& event) {
  auto item = m_list_view->get_list_item(index);
  if(!item) {
    return;
  }
  QApplication::setOverrideCursor(Qt::ClosedHandCursor);
  m_current_index = index;
  m_drag_index = index;
  m_list_view->grabMouse();
  m_drop_item = new DropItem(m_direction, m_list_view);
  m_drop_item->show();
  m_item_preview = make_item_preview(*item, m_list_view->window());
  m_item_preview->show();
  m_item_preview->raise();
  m_preview_offset = event.globalPos() - item->mapToGlobal(QPoint(0, 0));
  m_drag_item = item;
  m_drag_item->installEventFilter(this);
  m_list_view->get_current()->set(index);
  auto& selection = m_list_view->get_selection();
  selection->transact([&] {
    while(selection->get_size() > 0) {
      selection->remove(0);
    }
    selection->insert(index, 0);
  });
  update_drop_item();
}

void ListViewReorderController::stop_drag() {
  QApplication::restoreOverrideCursor();
  m_list_view->releaseMouse();
  if(m_drag_item) {
    m_drag_item->removeEventFilter(this);
    m_drag_item = nullptr;
  }
  if(m_item_preview) {
    m_item_preview->deleteLater();
    m_item_preview = nullptr;
  }
  if(m_drop_item) {
    m_drop_item->deleteLater();
    m_drop_item = nullptr;
  }
  m_current_index = -1;
  m_drag_index = -1;
  m_last_rect = {};
}

void ListViewReorderController::drag_move(const QMouseEvent& event) {
  if(!m_item_preview) {
    return;
  }
  auto global_pos = event.globalPos();
  auto preview_pos = m_item_preview->parentWidget()->mapFromGlobal(
    global_pos - m_preview_offset);
  m_item_preview->move(preview_pos);
  auto item_index = find_item_index(global_pos);
  if(item_index >= 0 && item_index != m_current_index) {
    if(item_index == m_drag_index && m_drag_index != m_current_index &&
        m_last_rect.contains(global_pos)) {
      return;
    }
    auto item = m_list_view->get_list_item(item_index);
    m_last_rect = QRect(item->mapToGlobal(QPoint(0, 0)), item->size());
    auto is_forward = item_index > m_current_index;
    m_drag_index = m_current_index;
    m_current_index = item_index;
    m_list_view->get_current()->set(boost::none);
    m_list_view->get_list()->move(m_drag_index, item_index);
    m_list_view->get_current()->set(m_current_index);
    m_drop_item->set_indicator_leading(!is_forward);
  }
}

void ListViewReorderController::keyboard_move(int source, int destination) {
  m_list_view->get_list()->move(source, destination);
  m_list_view->get_current()->set(destination);
}

void ListViewReorderController::update_drop_item() {
  if(!m_drag_item || !m_drop_item) {
    return;
  }
  m_drop_item->setFixedSize(m_drag_item->size());
  m_drop_item->move(m_drag_item->mapTo(m_list_view, QPoint(0, 0)));
}

void ListViewReorderController::on_style() {
  m_direction = Qt::Vertical;
  auto& stylist = find_stylist(*m_list_view);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (EnumProperty<Qt::Orientation> direction_style) {
        stylist.evaluate(direction_style, [&] (auto direction) {
          m_direction = direction;
        });
      });
  }
}
