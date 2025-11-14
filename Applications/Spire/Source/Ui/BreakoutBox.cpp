#include "Spire/Ui/BreakoutBox.hpp"
#include <QApplication>
#include <QWheelEvent>
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  template<bool (QWidget::* method)(bool)>
  struct FocusNext {
    friend bool focus_next(QWidget& widget, bool next) {
      return (widget.*method)(next);
    }
  };

  template struct FocusNext<&QWidget::focusNextPrevChild>;

  bool focus_next(QWidget& widget, bool next);

  struct BreakoutBody : QWidget {
    QWidget* m_source_parent;

    BreakoutBody(QWidget& body, QWidget& source_parent)
        : QWidget(&source_parent),
          m_source_parent(&source_parent) {
      enclose(*this, body);
      setFocusProxy(&body);
      proxy_style(*this, body);
    }

    bool focusNextPrevChild(bool next) override {
      if(parentWidget() == m_source_parent || !isVisible()) {
        return false;
      }
      auto focus_reason = [&] {
        if(next) {
          return Qt::FocusReason::TabFocusReason;
        }
        return Qt::FocusReason::BacktabFocusReason;
      }();
      m_source_parent->setFocus(focus_reason);
      return focus_next(*m_source_parent, next);
    }
  };
}

BreakoutBox::BreakoutBox(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_position_observer(*this) {
  m_body = new BreakoutBody(body, *this);
  enclose(*this, *m_body);
  setFocusProxy(m_body);
  proxy_style(*this, *m_body);
  m_position_observer.connect_position_signal(
    std::bind_front(&BreakoutBox::on_position, this));
}

QWidget& BreakoutBox::get_body() {
  return *m_body->layout()->itemAt(0)->widget();
}

bool BreakoutBox::is_broken_out() const {
  return layout()->count() == 0;;
}

void BreakoutBox::breakout() {
  if(is_broken_out()) {
    return;
  }
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
}

void BreakoutBox::restore() {
  if(!is_broken_out()) {
    return;
  }
  window()->removeEventFilter(this);
  qApp->removeEventFilter(this);
  m_body->removeEventFilter(this);
  m_body->setMinimumSize(0, 0);
  m_body->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
  layout()->addWidget(m_body);
  setFocusProxy(m_body);
  updateGeometry();
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
