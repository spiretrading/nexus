#include "Spire/Ui/BreakoutBox.hpp"
#include <QApplication>
#include <QWheelEvent>
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

struct BreakoutBox::BreakoutBody : QWidget {
  BreakoutBox* m_breakout_box;

  BreakoutBody(QWidget& body, BreakoutBox& breakout_box)
      : QWidget(&breakout_box),
        m_breakout_box(&breakout_box) {
    enclose(*this, body);
    setFocusProxy(&body);
    proxy_style(*this, body);
  }

  bool focusNextPrevChild(bool next) override {
    if(!m_breakout_box->is_broken_out() || m_breakout_box->m_is_transitioning) {
      return false;
    }
    auto focus_reason = [&] {
      if(next) {
        return Qt::FocusReason::TabFocusReason;
      }
      return Qt::FocusReason::BacktabFocusReason;
    }();
    m_breakout_box->setFocus(focus_reason);
    return focus_next(*m_breakout_box, next);
  }

  QWidget& get_body() {
    return *layout()->itemAt(0)->widget();
  }
};

BreakoutBox::BreakoutBox(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_is_transitioning(false),
      m_position_observer(*this) {
  m_body = new BreakoutBody(body, *this);
  enclose(*this, *m_body);
  setFocusProxy(m_body);
  proxy_style(*this, *m_body);
  m_position_observer.connect_position_signal(
    std::bind_front(&BreakoutBox::on_position, this));
}

QWidget& BreakoutBox::get_body() {
  return m_body->get_body();
}

bool BreakoutBox::is_broken_out() const {
  return layout()->count() == 0;;
}

void BreakoutBox::breakout() {
  if(is_broken_out()) {
    return;
  }
  m_is_transitioning = true;
  auto window = this->window();
  setFocusProxy(nullptr);
  layout()->removeWidget(m_body);
  updateGeometry();
  m_body->setParent(window);
  m_body->show();
  m_body->setFocus();
  m_body->move(window->mapFromGlobal(mapToGlobal(QPoint(0, 0))));
  adjust_size();
  window->installEventFilter(this);
  m_body->installEventFilter(this);
  qApp->installEventFilter(this);
  m_is_transitioning = false;
}

void BreakoutBox::restore() {
  if(!is_broken_out()) {
    return;
  }
  m_is_transitioning = true;
  window()->removeEventFilter(this);
  qApp->removeEventFilter(this);
  m_body->removeEventFilter(this);
  m_body->setMinimumSize(0, 0);
  m_body->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
  layout()->addWidget(m_body);
  setFocusProxy(m_body);
  updateGeometry();
  m_is_transitioning = false;
}

void BreakoutBox::resizeEvent(QResizeEvent* event) {
  if(is_broken_out()) {
    adjust_size();
  }
  QWidget::resizeEvent(event);
}

bool BreakoutBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body && event->type() == QEvent::LayoutRequest) {
    adjust_size();
  } else if(watched == window() && event->type() == QEvent::Resize) {
    adjust_size();
  } else if(event->type() == QEvent::Wheel && m_body->isVisible()) {
    auto& wheel_event = *static_cast<QWheelEvent*>(event);
    auto parent = m_body->parentWidget();
    auto global_pos = wheel_event.globalPos();
    if(parent->rect().contains(parent->mapFromGlobal(global_pos)) &&
        !m_body->rect().contains(m_body->mapFromGlobal(global_pos))) {
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void BreakoutBox::adjust_size() {
  update_size_constraints();
  invalidate_descendant_layouts(*m_body);
  m_body->adjustSize();
}

void BreakoutBox::update_size_constraints() {
  auto window = this->window();
  auto maximum_width = std::max(window->width() - m_body->x(), 0);
  auto maximum_height = std::max(window->height() - m_body->y(), 0);
  m_body->setMaximumSize(maximum_width, maximum_height);
  auto minimum_width = width();
  if(maximum_width <= minimum_width) {
    minimum_width = maximum_width;
  }
  auto minimum_height = height();
  if(maximum_height <= minimum_height) {
    minimum_height = maximum_height;
  }
  m_body->setMinimumSize(minimum_width, minimum_height);
}

void BreakoutBox::on_position(const QPoint& position) {
  if(is_broken_out()) {
    m_body->move(window()->mapFromGlobal(position));
  }
}
