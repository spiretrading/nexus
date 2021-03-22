#include "Spire/Ui/ScrollBox.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  int get_position(ScrollBar::Orientation orientation, const QPoint& point) {
    if(orientation == ScrollBar::Orientation::VERTICAL) {
      return point.y();
    }
    return point.x();
  }

  int get_position(ScrollBar::Orientation orientation, const QPointF& point) {
    if(orientation == ScrollBar::Orientation::VERTICAL) {
      return static_cast<int>(point.y());
    }
    return static_cast<int>(point.x());
  }

  int get_size(ScrollBar::Orientation orientation, const QSize& size) {
    if(orientation == ScrollBar::Orientation::VERTICAL) {
      return size.height();
    }
    return size.width();
  }
}

ScrollBar::ScrollBar(Orientation orientation, QWidget* parent)
    : StyledWidget(parent),
      m_orientation(orientation),
      m_is_dragging(false),
      m_thumb_start(0) {
  m_thumb = new Box(nullptr, nullptr);
  auto thumb_style = StyleSheet();
  thumb_style.get(Any()).set_override(Rule::Override::NONE).
    set(BackgroundColor(QColor("#C8C8C8")));
  m_thumb->set_style(std::move(thumb_style));
  m_thumb->setFixedHeight(scale_height(20));
  m_thumb->setSizePolicy(
    QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
  m_thumb->installEventFilter(this);
  m_track = new Box(m_thumb, this);
  auto track_style = m_track->get_style();
  track_style.get(Any()).set(border(0, QColor(0, 0, 0)));
  track_style.get(Any()).set(PaddingTop(m_thumb_start));
  track_style.get(Any()).set(PaddingRight(0));
  track_style.get(Any()).set(PaddingBottom(0));
  track_style.get(Any()).set(PaddingLeft(0));
  m_track->set_style(std::move(track_style));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_track);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

QSize ScrollBar::sizeHint() const {
  if(m_orientation == Orientation::VERTICAL) {
    return QSize(24, 1);
  } else {
    return QSize(1, 24);
  }
}

bool ScrollBar::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::MouseButtonPress) {
    auto& mouse_event = *static_cast<QMouseEvent*>(event);
    m_drag_position = get_position(m_orientation, mouse_event.windowPos());
    m_is_dragging = true;
  } else if(event->type() == QEvent::MouseButtonRelease) {
    m_is_dragging = false;
  } else if(event->type() == QEvent::MouseMove) {
    if(m_is_dragging) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      auto position = get_position(m_orientation, mouse_event.windowPos());
      auto delta = position - m_drag_position;
      if(position != m_drag_position) {
        delta = std::min(get_size(m_orientation, size()) -
          get_size(m_orientation, m_thumb->size()) - m_thumb_start,
          std::max(-m_thumb_start, delta));
        m_thumb_start += delta;
        auto track_style = m_track->get_style();
        track_style.get(Any()).set(PaddingTop(m_thumb_start));
        m_track->set_style(std::move(track_style));
        m_drag_position += delta;
      }
    }
  }
  return StyledWidget::eventFilter(watched, event);
}
