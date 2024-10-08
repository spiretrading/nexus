#include "Spire/Ui/PopupBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QEvent>
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

PopupBox::PopupBox(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_body(&body),
      m_window(nullptr),
      m_body_focus_observer(*m_body),
      m_focus_observer(*this),
      m_position_observer(*this),
      m_alignment(Alignment::NONE),
      m_max_height(QWIDGETSIZE_MAX),
      m_above_space(0),
      m_below_space(0),
      m_right_space(0) {
  m_body->installEventFilter(this);
  enclose(*this, *m_body);
  proxy_style(*this, *m_body);
  setFocusProxy(m_body);
  m_body_focus_observer.connect_state_signal(
    std::bind_front(&PopupBox::on_body_focus, this));
  m_focus_connection = m_focus_observer.connect_state_signal(
    std::bind_front(&PopupBox::on_focus, this));
  m_position_observer.connect_position_signal(
    std::bind_front(&PopupBox::on_position, this));
  m_min_height = [&] {
    if(m_body->minimumSizeHint().height() <= 0) {
      return m_body->sizeHint().height();
    }
    return m_body->minimumSizeHint().height();
  }();
}

const QWidget& PopupBox::get_body() const {
  return *m_body;
}

QWidget& PopupBox::get_body() {
  return *m_body;
}

QSize PopupBox::sizeHint() const {
  if(has_popped_up()) {
    return m_last_size;
  }
  return QWidget::sizeHint();
}

bool PopupBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_window) {
    if(event->type() == QEvent::Resize || event->type() == QEvent::Move) {
      update_space();
      if(has_popped_up()) {
        m_alignment = Alignment::NONE;
        align();
        adjust_size();
      }
    } else if(event->type() == QEvent::LayoutRequest && has_popped_up()) {
      update_space();
      adjust_size();
    }
  } else if(watched == m_body && event->type() == QEvent::Show) {
    update_window();
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

void PopupBox::align() {
  auto position = mapToGlobal(QPoint(0, 0));
  if(m_alignment == Alignment::ABOVE) {
    set_position(
      {position.x(), position.y() + m_min_height - m_body->height()});
    return;
  }
  if(m_body->sizeHint().height() >= m_below_space &&
      m_above_space > m_below_space) {
    m_alignment = Alignment::ABOVE;
    m_max_height = m_above_space;
    set_position(
      {position.x(), position.y() + m_min_height - m_body->height()});
  } else {
    m_alignment = Alignment::BELOW;
    m_max_height = m_below_space;
    set_position(position);
  }
}

void PopupBox::adjust_size() {
  auto size_policy = sizePolicy();
  if(size_policy.horizontalPolicy() == QSizePolicy::Preferred ||
      size_policy.horizontalPolicy() == QSizePolicy::Ignored) {
    if(m_body->sizeHint().width() > m_right_space) {
      m_body->setMinimumWidth(width());
      m_body->setMaximumWidth(m_right_space);
    } else if(m_body->sizeHint().width() < width() && width() > m_right_space) {
      m_body->setFixedWidth(m_right_space);
    } else {
      m_body->setMinimumWidth(width());
      m_body->setMaximumWidth(std::min(m_right_space, maximumWidth()));
    }
  } else if(size_policy.horizontalPolicy() == QSizePolicy::Expanding) {
    m_body->setFixedWidth(width());
  } else if(size_policy.horizontalPolicy() == QSizePolicy::Fixed) {
    m_body->setFixedWidth(m_last_size.width());
  }
  if(size_policy.verticalPolicy() == QSizePolicy::Preferred ||
      size_policy.verticalPolicy() == QSizePolicy::Ignored) {
    m_body->setMinimumHeight(m_min_height);
    m_body->setMaximumHeight(std::min(m_max_height, maximumHeight()));
  } else if(size_policy.verticalPolicy() == QSizePolicy::Expanding) {
    m_body->setFixedHeight(height());
  } else if(size_policy.verticalPolicy() == QSizePolicy::Fixed) {
    m_body->setFixedHeight(m_last_size.height());
  }
  m_body->adjustSize();
}

void PopupBox::set_position(const QPoint& pos) {
  m_body->move(m_window->mapFromGlobal(pos));
}

void PopupBox::update_window() {
  if(!m_window) {
    m_window = window();
    m_window->installEventFilter(this);
    update_space();
    m_body->setFixedHeight(m_min_height);
    adjustSize();
  }
}

void PopupBox::update_space() {
  if(!m_window) {
    return;
  }
  auto& window_rect = m_window->geometry();
  auto position = mapToGlobal(QPoint(0, 0));
  m_above_space = std::max(position.y() + m_min_height - window_rect.y(), 0);
  m_below_space = window_rect.bottom() - position.y();
  m_right_space = window_rect.right() - position.x();
}

void PopupBox::on_body_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE && has_popped_up()) {
    m_body->setFixedHeight(m_min_height);
    m_body->setMinimumWidth(0);
    m_body->setMaximumWidth(QWIDGETSIZE_MAX);
    layout()->addWidget(m_body);
    setFocusProxy(m_body);
  }
}

void PopupBox::on_focus(FocusObserver::State state) {
  if(state != FocusObserver::State::NONE && !has_popped_up()) {
    update_window();
    m_last_size = size();
    {
      auto blocker = shared_connection_block(m_focus_connection);
      setFocusProxy(nullptr);
      setFocus();
      m_body->setParent(m_window);
    }
    layout()->removeWidget(m_body);
    m_body->setMinimumSize(0, 0);
    m_body->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    m_body->show();
    m_body->setFocus();
    m_alignment = Alignment::NONE;
    align();
  }
}

void PopupBox::on_position(const QPoint& position) {
  if(has_popped_up()) {
    align();
  }
}
