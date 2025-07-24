#include "Spire/Ui/OverlayPanel.hpp"
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QScreen>
#include <QWindow>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/WindowObserver.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DROP_SHADOW_COLOR = QColor(0, 0, 0, 38);
  const auto DROP_SHADOW_OFFSET = QPoint(0, 0);
  const auto DROP_SHADOW_RADIUS = 5;
  const auto DROP_SHADOW_SIZE = 5;

  auto DROP_SHADOW_HEIGHT() {
    static auto height = scale_height(DROP_SHADOW_SIZE);
    return height;
  }

  auto DROP_SHADOW_WIDTH() {
    static auto width = scale_width(DROP_SHADOW_SIZE);
    return width;
  }

  auto DROP_SHADOW_MARGINS() {
    static auto margins = QMargins(DROP_SHADOW_WIDTH(), DROP_SHADOW_HEIGHT(),
      DROP_SHADOW_WIDTH(), DROP_SHADOW_HEIGHT());
    return margins;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(0xA0A0A0)));
    return style;
  }
}

OverlayPanel::OverlayPanel(QWidget& body, QWidget& parent)
    : QWidget(&parent,
        Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
      m_body(&body),
      m_is_closed_on_focus_out(true),
      m_is_draggable(true),
      m_was_activated(false),
      m_positioning(Positioning::PARENT),
      m_parent_focus_observer(parent),
      m_focus_observer(*this),
      m_parent_position_observer(parent) {
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_QuitOnClose);
  auto box = new Box(m_body);
  box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setFocusProxy(box);
  auto layout = make_vbox_layout(this);
  layout->setContentsMargins(DROP_SHADOW_MARGINS());
  layout->addWidget(box);
  layout->addSpacerItem(
    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  auto shadow = new QGraphicsDropShadowEffect();
  shadow->setColor(DROP_SHADOW_COLOR);
  shadow->setOffset(translate(DROP_SHADOW_OFFSET));
  shadow->setBlurRadius(scale_width(DROP_SHADOW_RADIUS));
  box->setGraphicsEffect(shadow);
  m_focus_connection = m_focus_observer.connect_state_signal(
    std::bind_front(&OverlayPanel::on_focus, this));
  m_parent_focus_connection = m_parent_focus_observer.connect_state_signal(
    std::bind_front(&OverlayPanel::on_parent_focus, this));
  m_parent_position_connection =
    m_parent_position_observer.connect_position_signal(
      std::bind_front(&OverlayPanel::on_parent_move, this));
  m_body->installEventFilter(this);
  parent.installEventFilter(this);
}

const QWidget& OverlayPanel::get_body() const {
  return *m_body;
}

QWidget& OverlayPanel::get_body() {
  return *m_body;
}

bool OverlayPanel::is_closed_on_focus_out() const {
  return m_is_closed_on_focus_out;
}

void OverlayPanel::set_closed_on_focus_out(bool is_closed_on_focus_out) {
  m_is_closed_on_focus_out = is_closed_on_focus_out;
}

bool OverlayPanel::is_draggable() const {
  return m_is_draggable;
}

void OverlayPanel::set_is_draggable(bool is_draggable) {
  m_is_draggable = is_draggable;
}

OverlayPanel::Positioning OverlayPanel::get_positioning() const {
  return m_positioning;
}

void OverlayPanel::set_positioning(Positioning positioning) {
  m_positioning = positioning;
}

bool OverlayPanel::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body) {
    if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = static_cast<QMouseEvent&>(*event);
      m_mouse_pressed_position = mouse_event.pos();
    } else if(event->type() == QEvent::MouseMove && m_is_draggable &&
        m_positioning != Positioning::PARENT) {
      auto& mouse_event = static_cast<QMouseEvent&>(*event);
      if(mouse_event.buttons() & Qt::LeftButton) {
        move(pos() + (mouse_event.pos() - m_mouse_pressed_position));
      }
    } else if(event->type() == QEvent::LayoutRequest) {
      setFixedSize(layout()->itemAt(0)->widget()->sizeHint().grownBy(
        layout()->contentsMargins()));
    }
  } else if(watched == parentWidget() && event->type() == QEvent::Resize) {
    if(isVisible()) {
      position();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void OverlayPanel::showEvent(QShowEvent* event) {
  position();
  QWidget::showEvent(event);
}

void OverlayPanel::closeEvent(QCloseEvent* event) {
  m_was_activated = false;
  if(auto focused_widget = focusWidget()) {
    focused_widget->clearFocus();
  }
  QWidget::closeEvent(event);
}

void OverlayPanel::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    close();
    return;
  }
  QWidget::keyPressEvent(event);
}

void OverlayPanel::resizeEvent(QResizeEvent* event) {
  update_mask();
  position();
  QWidget::resizeEvent(event);
}

void OverlayPanel::position() {
  if(m_positioning == Positioning::PARENT) {
    auto parent_geometry = parentWidget()->rect();
    auto parent_bottom_left = parentWidget()->mapToGlobal(
      parent_geometry.bottomLeft());
    auto screen_geometry = parentWidget()->screen()->availableGeometry();
    auto panel_size = size();
    auto x = [&] {
      auto x = parent_bottom_left.x() - DROP_SHADOW_WIDTH();
      if(x < screen_geometry.left()) {
        return screen_geometry.left() - DROP_SHADOW_WIDTH();
      } else if(x + panel_size.width() > screen_geometry.right()) {
        return screen_geometry.right() - panel_size.width() +
          DROP_SHADOW_WIDTH();
      }
      return x;
    }();
    auto pos = [&] {
      if((parent_bottom_left.y() + panel_size.height()) >
          screen_geometry.bottom()) {
        return QPoint(x, parent_bottom_left.y() - parent_geometry.height() -
          panel_size.height() + 1 + DROP_SHADOW_HEIGHT());
      }
      return QPoint(x, parent_bottom_left.y() + 1 - DROP_SHADOW_HEIGHT());
    }();
    move(pos);
    update();
    update_mask();
  }
}

void OverlayPanel::on_focus(FocusObserver::State state) {
  if(m_is_closed_on_focus_out) {
    if(state == FocusObserver::State::NONE) {
      close();
    } else if(state == FocusObserver::State::FOCUS_IN) {
      m_was_activated = true;
    }
  }
}

void OverlayPanel::on_parent_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE && m_is_closed_on_focus_out &&
      !isActiveWindow() && m_was_activated) {
    close();
  } else if(state == FocusObserver::State::FOCUS_IN &&
      m_is_closed_on_focus_out && m_was_activated) {
    close();
  }
}

void OverlayPanel::on_parent_move(QPoint) {
  if(isVisible()) {
    position();
  }
}

void OverlayPanel::update_mask() {
  if(m_positioning == Positioning::PARENT) {
    auto intersection = geometry().intersected(
      QRect(parentWidget()->mapToGlobal(QPoint()), parentWidget()->size()));
    setMask(QPolygon(rect()).subtracted(
      QRect(mapFromGlobal(intersection.topLeft()), intersection.size())));
  }
}
