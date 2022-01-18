#include "Spire/Ui/ScrollBar.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include "Spire/Ui/Box.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto INITIAL_TRACK_SCROLL_TIMEOUT_MS = 300;
  const auto REPEAT_TRACK_SCROLL_TIMEOUT_MS = 50;

  int get_position(Qt::Orientation orientation, const QPoint& point) {
    if(orientation == Qt::Orientation::Vertical) {
      return point.y();
    }
    return point.x();
  }

  int get_position(Qt::Orientation orientation, const QPointF& point) {
    if(orientation == Qt::Orientation::Vertical) {
      return static_cast<int>(point.y());
    }
    return static_cast<int>(point.x());
  }

  int get_size(Qt::Orientation orientation, const QSize& size) {
    if(orientation == Qt::Orientation::Vertical) {
      return size.height();
    }
    return size.width();
  }
}

ScrollBar::ScrollBar(Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent),
      m_orientation(orientation),
      m_range(0, 100),
      m_line_size(1),
      m_page_size(20),
      m_position(0),
      m_is_dragging(false),
      m_thumb_position(0),
      m_track_scroll_direction(0),
      m_track_scroll_timer(this) {
  m_thumb = new Box(nullptr, nullptr);
  update_style(*m_thumb, [&] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
  });
  if(m_orientation == Qt::Orientation::Vertical) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_thumb->setSizePolicy(
      QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
  } else {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_thumb->setSizePolicy(
      QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
  }
  m_track = new Box(m_thumb, this);
  update_style(*m_track, [&] (auto& style) {
    style.get(Any()).
      set(border(0, QColor(Qt::black))).
      set(BackgroundColor(QColor(0xFFFFFF)));
    if(m_orientation == Qt::Orientation::Vertical) {
      style.get(Any()).set(PaddingTop(m_thumb_position));
      style.get(Any()).set(PaddingLeft(0));
    } else {
      style.get(Any()).set(PaddingTop(0));
      style.get(Any()).set(PaddingLeft(m_thumb_position));
    }
    style.get(Any()).set(PaddingRight(0));
    style.get(Any()).set(PaddingBottom(0));
  });
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_track);
  m_track_scroll_timer.setSingleShot(true);
  connect(
    &m_track_scroll_timer, &QTimer::timeout, this, &ScrollBar::scroll_page);
  update_thumb();
}

Qt::Orientation ScrollBar::get_orientation() {
  return m_orientation;
}

ScrollBar::Range ScrollBar::get_range() const {
  return m_range;
}

void ScrollBar::set_range(const Range& range) {
  m_range = range;
  if(m_position < m_range.m_start) {
    set_position(m_range.m_start);
  } else if(m_position > m_range.m_end) {
    set_position(m_range.m_end);
  } else {
    update_thumb();
  }
}

void ScrollBar::set_range(int start, int end) {
  return set_range({start, end});
}

int ScrollBar::get_line_size() const {
  return m_line_size;
}

void ScrollBar::set_line_size(int size) {
  m_line_size = size;
}

int ScrollBar::get_page_size() const {
  return m_page_size;
}

void ScrollBar::set_page_size(int size) {
  m_page_size = size;
  update_thumb();
}

int ScrollBar::get_position() const {
  return m_position;
}

void ScrollBar::set_position(int position) {
  position = std::clamp(position, m_range.m_start, m_range.m_end);
  if(position == m_position) {
    return;
  }
  m_position = position;
  update_thumb();
  m_position_signal(m_position);
}

connection ScrollBar::connect_position_signal(
    const PositionSignal::slot_type& slot) const {
  return m_position_signal.connect(slot);
}

QSize ScrollBar::sizeHint() const {
  if(m_orientation == Qt::Orientation::Vertical) {
    return scale(15, 1);
  } else {
    return scale(1, 15);
  }
}

void ScrollBar::mouseMoveEvent(QMouseEvent* event) {
  if(m_is_dragging) {
    auto position = ::get_position(m_orientation, event->windowPos());
    if(position != m_drag_position) {
      auto region = get_size(m_orientation, size()) -
        get_size(m_orientation, m_thumb->size());
      if(region >= 0) {
        auto delta = std::clamp(position - m_drag_position,
          -m_thumb_position, region - m_thumb_position);
        auto updated_thumb_position =
          static_cast<double>(m_thumb_position + delta);
        set_position(static_cast<int>(std::ceil(
          (updated_thumb_position * (m_range.m_end - m_range.m_start) +
            region * m_range.m_start) / region)));
        m_drag_position += delta;
      }
    }
  }
  QWidget::mouseMoveEvent(event);
}

void ScrollBar::mousePressEvent(QMouseEvent* event) {
  if(m_thumb->rect().contains(m_thumb->mapFromGlobal(event->globalPos()))) {
    m_drag_position = ::get_position(m_orientation, event->windowPos());
    m_is_dragging = true;
  } else if(::get_position(m_orientation, event->globalPos()) < ::get_position(
      m_orientation, m_thumb->mapToGlobal(m_thumb->pos()))) {
    m_track_scroll_direction = -1;
    scroll_page_up(*this);
    m_track_scroll_timer.setInterval(INITIAL_TRACK_SCROLL_TIMEOUT_MS);
    m_track_scroll_timer.start();
  } else {
    m_track_scroll_direction = 1;
    scroll_page_down(*this);
    m_track_scroll_timer.setInterval(INITIAL_TRACK_SCROLL_TIMEOUT_MS);
    m_track_scroll_timer.start();
  }
  QWidget::mousePressEvent(event);
}

void ScrollBar::mouseReleaseEvent(QMouseEvent* event) {
  m_is_dragging = false;
  if(m_track_scroll_direction != 0) {
    m_track_scroll_timer.stop();
    m_track_scroll_direction = 0;
  }
  QWidget::mouseReleaseEvent(event);
}

void ScrollBar::resizeEvent(QResizeEvent* event) {
  update_thumb();
}

void ScrollBar::update_thumb() {
  auto track_size = get_size(m_orientation, size());
  auto thumb_size = get_size(m_orientation, m_thumb->size());
  auto region = m_page_size + m_range.m_end - m_range.m_start + 1;
  auto min_size = [&] {
    if(m_orientation == Qt::Orientation::Vertical) {
      return scale_height(20);
    }
    return scale_width(20);
  }();
  auto expected_size = std::max(static_cast<int>(std::ceil(
    static_cast<double>(track_size) * m_page_size / region)), min_size);
  if(thumb_size != expected_size) {
    thumb_size = expected_size;
    if(m_orientation == Qt::Orientation::Vertical) {
      m_thumb->setFixedHeight(thumb_size);
    } else {
      m_thumb->setFixedWidth(thumb_size);
    }
  }
  if(m_range.m_end <= m_range.m_start) {
    m_thumb_position = 0;
  } else {
    m_thumb_position = (track_size - thumb_size) *
      (m_position - m_range.m_start) / (m_range.m_end - m_range.m_start);
  }
  update_style(*m_track, [&] (auto& style) {
    if(m_orientation == Qt::Orientation::Vertical) {
      style.get(Any()).set(PaddingTop(m_thumb_position));
    } else {
      style.get(Any()).set(PaddingLeft(m_thumb_position));
    }
  });
  update();
}

void ScrollBar::scroll_page() {
  if(m_track->rect().contains(m_track->mapFromGlobal(QCursor::pos()))) {
    auto cursor_position = ::get_position(m_orientation, QCursor::pos());
    auto thumb_position =
      ::get_position(m_orientation, m_thumb->mapToGlobal(m_thumb->pos()));
    if(m_track_scroll_direction == -1) {
      if(cursor_position < thumb_position) {
        scroll_page_up(*this);
      }
    } else if(m_track_scroll_direction == 1) {
      if(cursor_position >
          thumb_position + get_size(m_orientation, m_thumb->size())) {
        scroll_page_down(*this);
      }
    }
  }
  m_track_scroll_timer.setInterval(REPEAT_TRACK_SCROLL_TIMEOUT_MS);
  m_track_scroll_timer.start();
}

void Spire::scroll_line_up(ScrollBar& scroll_bar, int lines) {
  scroll_bar.set_position(
    scroll_bar.get_position() - lines * scroll_bar.get_line_size());
}

void Spire::scroll_line_down(ScrollBar& scroll_bar, int lines) {
  scroll_bar.set_position(
    scroll_bar.get_position() + lines * scroll_bar.get_line_size());
}

void Spire::scroll_page_up(ScrollBar& scroll_bar, int pages) {
  scroll_bar.set_position(
    scroll_bar.get_position() - pages * scroll_bar.get_page_size());
}

void Spire::scroll_page_down(ScrollBar& scroll_bar, int pages) {
  scroll_bar.set_position(
    scroll_bar.get_position() + pages * scroll_bar.get_page_size());
}

void Spire::scroll_to_start(ScrollBar& scroll_bar) {
  scroll_bar.set_position(scroll_bar.get_range().m_start);
}

void Spire::scroll_to_end(ScrollBar& scroll_bar) {
  scroll_bar.set_position(scroll_bar.get_range().m_end);
}
