#include "Spire/Ui/OverlayPanel.hpp"
#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QScreen>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DROP_SHADOW_COLOR = QColor(0, 0, 0, 64);
  const auto DROP_SHADOW_OFFSET = QPoint(0, 3);
  const auto DROP_SHADOW_RADIUS = 5;
  const auto DROP_SHADOW_SIZE = 3;
  const auto FADE_SPEED_MS = 100;

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
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border(scale_width(1), QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    return style;
  }

  QWidget* find_top_level_window() {
    auto list = QApplication::topLevelWidgets();
    for(auto i = 0; i < list.size(); ++i) {
      auto widget = list.at(i);
      if(widget->isVisible() && !widget->parentWidget() &&
          widget->windowFlags() & Qt::Window) {
        return widget;
      }
    }
    return nullptr;
  }
}

OverlayPanel::OverlayPanel(QWidget* body, QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint |
        Qt::NoDropShadowWindowHint),
      m_body(body),
      m_top_level_window(nullptr),
      m_is_closed_on_blur(true),
      m_is_closed(false),
      m_positioning(Positioning::PARENT) {
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_QuitOnClose);
  auto box = new Box(m_body);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(DROP_SHADOW_MARGINS());
  layout->addWidget(box);
  proxy_style(*this, *box);
  set_style(*box, DEFAULT_STYLE());
  auto shadow = new QGraphicsDropShadowEffect();
  shadow->setColor(DROP_SHADOW_COLOR);
  shadow->setOffset(translate(DROP_SHADOW_OFFSET));
  shadow->setBlurRadius(scale_width(DROP_SHADOW_RADIUS));
  box->setGraphicsEffect(shadow);
  m_body->installEventFilter(this);
  if(m_top_level_window = find_top_level_window()) {
    m_top_level_window->installEventFilter(this);
  }
}

const QWidget& OverlayPanel::get_body() const {
  return *m_body;
}

QWidget& OverlayPanel::get_body() {
  return *m_body;
}

bool OverlayPanel::is_closed_on_blur() const {
  return m_is_closed_on_blur;
}

void OverlayPanel::set_closed_on_blur(bool is_closed_on_blur) {
  m_is_closed_on_blur = is_closed_on_blur;
}

OverlayPanel::Positioning OverlayPanel::get_positioning() const {
  return m_positioning;
}

void OverlayPanel::set_positioning(Positioning positioning) {
  m_positioning = positioning;
}

connection OverlayPanel::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

bool OverlayPanel::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body) {
    if(event->type() == QEvent::Resize) {
      resize(m_body->size().grownBy(DROP_SHADOW_MARGINS()));
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto mouse_event = static_cast<QMouseEvent*>(event);
      m_mouse_press_position = mouse_event->pos();
    } else if(event->type() == QEvent::MouseMove) {
      auto mouse_event = static_cast<QMouseEvent*>(event);
      if(mouse_event->buttons() & Qt::LeftButton &&
          m_positioning != Positioning::PARENT) {
        move(mapToGlobal(mouse_event->pos() - m_mouse_press_position));
      }
    }
  } else if(watched == m_top_level_window) {
    if(event->type() == QEvent::Move) {
      position();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void OverlayPanel::showEvent(QShowEvent* event) {
  position();
  fade(false);
  activateWindow();
  m_is_closed = false;
  QWidget::showEvent(event);
}

void OverlayPanel::closeEvent(QCloseEvent* event) {
  if(windowOpacity() == 1.0) {
    fade(true);
    event->ignore();
  } else {
    event->accept();
    m_is_closed = true;
  }
}

bool OverlayPanel::event(QEvent* event) {
  if(event->type() == QEvent::WindowDeactivate) {
    if(m_is_closed_on_blur && !m_is_closed) {
      fade(true);
    }
  }
  return QWidget::event(event);
}


QScreen* OverlayPanel::get_current_screen(const QPoint& point) const {
  if(auto screen = QGuiApplication::screenAt(point)) {
    return screen;
  }
  return parentWidget()->screen();
}

void OverlayPanel::fade(bool reverse) {
  auto animation = new QPropertyAnimation(this, "windowOpacity");
  animation->setDuration(FADE_SPEED_MS);
  animation->setEasingCurve(QEasingCurve::Linear);
  if(!reverse) {
    animation->setStartValue(0);
    animation->setEndValue(1);
  } else {
    animation->setStartValue(1);
    animation->setEndValue(0);
    connect(animation, &QPropertyAnimation::finished, [=] {
      close();
    });
  }
  animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void OverlayPanel::position() {
  if(m_positioning == Positioning::PARENT) {
    auto parent_geometry = parentWidget()->rect();
    auto parent_bottom_left = parentWidget()->mapToGlobal(
      parent_geometry.bottomLeft());
    auto screen_geometry =
      get_current_screen(parent_bottom_left)->availableGeometry();
    auto get_x = [&] {
      auto x = parent_bottom_left.x() - DROP_SHADOW_WIDTH();
      if(x < 0) {
        return 0;
      } else if(x + width() > screen_geometry.right()) {
        return screen_geometry.right() - width();
      }
      return x;
    };
    if((parent_bottom_left.y() + height()) > screen_geometry.bottom()) {
      layout()->setContentsMargins(DROP_SHADOW_WIDTH(), DROP_SHADOW_HEIGHT(),
        DROP_SHADOW_WIDTH(), 0);
      update();
      move({get_x(), parent_bottom_left.y() - parent_geometry.height() -
        height() + scale_height(1)});
    } else {
      layout()->setContentsMargins(DROP_SHADOW_WIDTH(), 0,
        DROP_SHADOW_WIDTH(), DROP_SHADOW_HEIGHT());
      update();
      move({get_x(), parent_bottom_left.y() + scale_height(1)});
    }
  }
}
