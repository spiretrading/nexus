#include "Spire/Ui/FocusPopupBox.hpp"
#include <QEvent>
#include <QResizeEvent>
#include "Spire/Ui/Layouts.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto is_widget_in_layout(const QLayout& layout, QWidget* widget) {
    for(auto i = 0; i < layout.count(); ++i) {
      auto item = layout.itemAt(i);
      if(!item) {
        continue;
      }
      if(item->widget() == widget) {
        return true;
      }
      if(item->layout()) {
        return is_widget_in_layout(*item->layout(), widget);
      }
    }
    return false;
  }
}

FocusPopupBox::FocusPopupBox(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_body(&body),
      m_window(nullptr),
      m_focus_observer(*this),
      m_body_focus_observer(*m_body),
      m_position_observer(*this),
      m_alignment(Alignment::NONE),
      m_last_alignment(m_alignment),
      m_min_height(m_body->sizeHint().height()),
      m_max_height(QWIDGETSIZE_MAX),
      m_above_space(0),
      m_below_space(0),
      m_focus_connection(m_focus_observer.connect_state_signal(
        std::bind_front(&FocusPopupBox::on_focus, this))),
      m_body_focus_connection(m_body_focus_observer.connect_state_signal(
        std::bind_front(&FocusPopupBox::on_body_focus, this))),
      m_position_connection(m_position_observer.connect_position_signal(
        std::bind_front(&FocusPopupBox::on_position, this))) {
  m_body->installEventFilter(this);
  m_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *m_body);
}

const QWidget& FocusPopupBox::get_body() const {
  return *m_body;
}

QWidget& FocusPopupBox::get_body() {
  return *m_body;
}

QSize FocusPopupBox::sizeHint() const {
  if(layout()->count() == 0) {
    return m_body->sizeHint();
  }
  return layout()->sizeHint();
}

bool FocusPopupBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_window && event->type() == QEvent::Resize) {
    update_space();
    if(layout()->count() == 0 &&
        m_body_focus_observer.get_state() != FocusObserver::State::NONE) {
      m_alignment = Alignment::NONE;
      align();
      adjust_size();
    }
  } else if(watched == m_body) {
    if(event->type() == QEvent::Show) {
      if(!m_window) {
        m_window = window();
        m_window->installEventFilter(this);
        update_space();
      }
    } else if(event->type() == QEvent::Resize && layout()->count() == 0 &&
        m_focus_observer.get_state() != FocusObserver::State::NONE) {
      align();
    } else if(event->type() == QEvent::LayoutRequest &&
        layout()->count() == 0) {
      adjust_size();
    }
  }
  return QObject::eventFilter(watched, event);
}

void FocusPopupBox::resizeEvent(QResizeEvent* event) {
  if(layout()->count() == 0) {
    m_body->setFixedWidth(event->size().width());
    align();
    adjust_size();
  }
  QWidget::resizeEvent(event);
}

void FocusPopupBox::on_focus(FocusObserver::State state) {
  if(state != FocusObserver::State::NONE && layout()->count() > 0) {
    layout()->removeWidget(m_body);
    m_body->setParent(m_window);
    m_body->raise();
    m_body->show();
    m_body->setFixedWidth(width());
    m_body->setFocus();
    m_alignment = Alignment::NONE;
    m_last_alignment = m_alignment;
    align();
    parentWidget()->clearFocus();
  }
}

void FocusPopupBox::on_body_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE && layout()->count() == 0) {
    m_body->setMinimumSize(0, 0);
    m_body->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    layout()->addWidget(m_body);
  }
}

void FocusPopupBox::on_position(const QPoint& position) {
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
  if(layout()->count() == 0) {
    set_position(m_position);
  }
}

void FocusPopupBox::align() {
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
    if(m_last_alignment != m_alignment) {
      set_position(m_position);
    }
  }
  m_last_alignment = m_alignment;
}

void FocusPopupBox::adjust_size() {
  if(minimumHeight() == maximumHeight()) {
    m_body->setMaximumHeight(QWIDGETSIZE_MAX);
    return;
  }
  auto height = [&] {
    auto max_height = std::min(m_max_height, maximumHeight());
    m_body->setMaximumHeight(max_height);
    if(m_body->sizeHint().height() >= max_height) {
      return max_height;
    }
    return m_body->sizeHint().height();
  }();
  m_body->resize(m_body->width(), height);
}

void FocusPopupBox::set_position(const QPoint& pos) {
  m_body->move(m_window->mapFromGlobal(pos));
}

void FocusPopupBox::update_space() {
  if(!m_window) {
    return;
  }
  auto& window_rect = m_window->geometry();
  m_above_space = m_position.y() + m_min_height - window_rect.y();
  m_above_space = std::max(m_above_space, 0);
  m_below_space = window_rect.bottom() - m_position.y();
}
