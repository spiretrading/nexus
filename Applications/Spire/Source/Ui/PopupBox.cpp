#include "Spire/Ui/PopupBox.hpp"
#include <QApplication>
#include <QEvent>
#include <QResizeEvent>
#include "Spire/Ui/Layouts.hpp"

using namespace boost::signals2;
using namespace Spire;

PopupBox::PopupBox(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_body(&body),
      m_window(nullptr),
      m_body_focus_observer(*m_body),
      m_position_observer(*this),
      m_alignment(Alignment::NONE),
      m_min_height(m_body->sizeHint().height()),
      m_max_height(QWIDGETSIZE_MAX),
      m_above_space(0),
      m_below_space(0),
      m_right_space(0),
      m_body_focus_connection(m_body_focus_observer.connect_state_signal(
        std::bind_front(&PopupBox::on_body_focus, this))),
      m_position_connection(m_position_observer.connect_position_signal(
        std::bind_front(&PopupBox::on_position, this))) {
  m_body->installEventFilter(this);
  m_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *m_body);
}

const QWidget& PopupBox::get_body() const {
  return *m_body;
}

QWidget& PopupBox::get_body() {
  return *m_body;
}

QSize PopupBox::sizeHint() const {
  if(has_popped_up()) {
    return m_size_hint;
  }
  m_size_hint = m_body->sizeHint();
  return QWidget::sizeHint();
}

bool PopupBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_window) {
    if(event->type() == QEvent::Resize) {
      update_space();
      if(has_popped_up()) {
        m_alignment = Alignment::NONE;
        align();
        adjust_size();
      }
    } else if(event->type() == QEvent::LayoutRequest && has_popped_up()) {
      adjust_size();
    }
  } else if(watched == m_body) {
    if(event->type() == QEvent::Show) {
      if(!m_window) {
        m_window = window();
        m_window->installEventFilter(this);
        update_space();
      }
    } else if(event->type() == QEvent::LayoutRequest && has_popped_up()) {
      align();
      adjust_size();
    }
  }
  return QObject::eventFilter(watched, event);
}

void PopupBox::resizeEvent(QResizeEvent* event) {
  if(has_popped_up()) {
    align();
    adjust_size();
  }
  QWidget::resizeEvent(event);
}

bool PopupBox::has_popped_up() const {
  return layout()->count() == 0;
}

void PopupBox::on_body_focus(FocusObserver::State state) {
  static auto is_changing_parent = false;
  if(is_changing_parent) {
    return;
  }
  if(state != FocusObserver::State::NONE && !has_popped_up()) {
    layout()->removeWidget(m_body);
    is_changing_parent = true;
    m_body->setParent(m_window);
    is_changing_parent = false;
    m_body->show();
    m_body->setFocus();
    m_alignment = Alignment::NONE;
    align();
  } else if(state == FocusObserver::State::NONE &&
      find_focus_state(*this) == FocusObserver::State::NONE &&
      has_popped_up() && QApplication::focusWidget()) {
    m_body->setMinimumSize(0, 0);
    m_body->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    layout()->addWidget(m_body);
  }
}

void PopupBox::on_position(const QPoint& position) {
  m_position = position;
  if(m_window) {
    auto& window_rect = m_window->geometry();
    if(m_position.x() < window_rect.x()) {
      m_position.setX(window_rect.x());
    }
    if(m_position.y() < window_rect.y()) {
      m_position.setY(window_rect.y());
    }
  }
  if(has_popped_up()) {
    align();
  }
}

void PopupBox::align() {
  if(m_alignment == Alignment::ABOVE) {
    set_position(
      {m_position.x(), m_position.y() + m_min_height - m_body->height()});
    return;
  }
  if(m_body->sizeHint().height() >= m_below_space &&
      m_above_space > m_below_space) {
    m_alignment = Alignment::ABOVE;
    m_max_height = m_above_space;
    set_position(
      {m_position.x(), m_position.y() + m_min_height - m_body->height()});
  } else {
    m_alignment = Alignment::BELOW;
    m_max_height = m_below_space;
    set_position(m_position);
  }
}

void PopupBox::adjust_size() {
  if(minimumHeight() == maximumHeight()) {
    m_body->setMaximumHeight(QWIDGETSIZE_MAX);
    return;
  }
  m_body->setMaximumSize(std::min(m_right_space, maximumWidth()),
    std::min(m_max_height, maximumHeight()));
  m_body->setMinimumWidth(width());
  m_body->adjustSize();
}

void PopupBox::set_position(const QPoint& pos) {
  m_body->move(m_window->mapFromGlobal(pos));
}

void PopupBox::update_space() {
  if(!m_window) {
    return;
  }
  auto& window_rect = m_window->geometry();
  m_above_space = m_position.y() + m_min_height - window_rect.y();
  m_above_space = std::max(m_above_space, 0);
  m_below_space = window_rect.bottom() - m_position.y();
  m_right_space = window_rect.right() - m_position.x();
}
