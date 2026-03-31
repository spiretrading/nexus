#include "Spire/Ui/PopupBox.hpp"
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/BoxPainter.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DROP_SHADOW_COLOR = QColor(0, 0, 0, 38);
  const auto DROP_SHADOW_RADIUS = 5;
  const auto DROP_SHADOW_SIZE = 5;

  struct PopupPanel : QWidget {
    QWidget* m_panel;

    explicit PopupPanel(QWidget& panel, QWidget* parent = nullptr)
        : QWidget(parent, Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_panel(&panel) {
      setAttribute(Qt::WA_TranslucentBackground);
      enclose(*this, *m_panel);
      proxy_style(*this, *m_panel);
      setFocusProxy(m_panel);
    }

    QSize minimumSizeHint() const override {
      return m_panel->minimumSizeHint();
    }

    QSize sizeHint() const override {
      return m_panel->sizeHint();
    }

    void showEvent(QShowEvent* event) override {
      if(isWindow()) {
        update_style(*m_panel, [] (auto& style) {
          style.get(Any()).set(border_color(QColor(0xA0A0A0)));
        });
        auto size = scale(DROP_SHADOW_SIZE, DROP_SHADOW_SIZE);
        layout()->setContentsMargins(
          size.width(), size.height(), size.width(), size.height());
        auto shadow = new QGraphicsDropShadowEffect();
        shadow->setColor(DROP_SHADOW_COLOR);
        shadow->setOffset(translate(QPoint(0, 0)));
        shadow->setBlurRadius(scale_width(DROP_SHADOW_RADIUS));
        m_panel->setGraphicsEffect(shadow);
      } else {
        update_style(*m_panel, [] (auto& style) {
          style.get(Any()).set(border_color(QColor(Qt::transparent)));
        });
        layout()->setContentsMargins({});
        m_panel->setGraphicsEffect(nullptr);
      }
      QWidget::showEvent(event);
    }
  };
}

PopupBox::PopupBox(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_body(&body),
      m_overflow_directions(Qt::Horizontal | Qt::Vertical),
      m_position_observer(*this) {
  enclose(*this, *m_body);
  proxy_style(*this, *m_body);
  setFocusProxy(m_body);
  m_body->installEventFilter(this);
  m_position_observer.connect_position_signal(
    std::bind_front(&PopupBox::on_position, this));
}

const QWidget& PopupBox::get_body() const {
  return *m_body;
}

QWidget& PopupBox::get_body() {
  return *m_body;
}

Qt::Orientations PopupBox::get_overflow_directions() const {
  return m_overflow_directions;
}

void PopupBox::set_overflow_directions(Qt::Orientations directions) {
  if(m_overflow_directions == directions) {
    return;
  }
  m_overflow_directions = directions;
  handle_overflow();
}

QSize PopupBox::sizeHint() const {
  if(is_popped_up()) {
    return m_body_size;
  }
  return QWidget::sizeHint();
}

bool PopupBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body && event->type() == QEvent::LayoutRequest) {
    handle_overflow();
  } else if(watched == window() && event->type() == QEvent::WindowDeactivate &&
      m_body->isActiveWindow()) {
    return true;
  }
  return QWidget::eventFilter(watched, event);
}

void PopupBox::resizeEvent(QResizeEvent* event) {
  handle_overflow();
  QWidget::resizeEvent(event);
}

bool PopupBox::is_popped_up() const {
  return layout()->count() == 0;
}

void PopupBox::popup() {
  if(is_popped_up()) {
    return;
  }
  m_body_size = m_body->sizeHint();
  window()->installEventFilter(this);
  setFocusProxy(nullptr);
  layout()->removeWidget(m_body);
  m_body->setWindowFlags(m_body->windowFlags() | Qt::Tool);
  m_body->show();
  m_body->move(mapToGlobal(QPoint(0, 0)));
  m_body->adjustSize();
  m_body->activateWindow();
}

void PopupBox::restore() {
  if(!is_popped_up()) {
    return;
  }
  window()->removeEventFilter(this);
  m_body->setWindowFlags(m_body->windowFlags() & ~Qt::Tool);
  layout()->addWidget(m_body);
  setFocusProxy(m_body);
  updateGeometry();
}

void PopupBox::handle_overflow() {
  auto size = this->size();
  auto body_size = m_body->sizeHint();
  auto overflows =
    ((m_overflow_directions & Qt::Horizontal) &&
      size.width() < body_size.width()) ||
    ((m_overflow_directions & Qt::Vertical) &&
      size.height() < body_size.height());
  if(overflows) {
    popup();
  } else {
    restore();
  }
}

void PopupBox::on_position(const QPoint& position) {
  if(is_popped_up()) {
    m_body->move(position);
  }
}

PopupBox* Spire::make_popup_panel(QWidget& panel, QWidget* parent) {
  return new PopupBox(*new PopupPanel(panel), parent);
}
