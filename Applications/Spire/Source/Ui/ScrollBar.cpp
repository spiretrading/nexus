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
    : StyledWidget(parent),
      m_orientation(orientation),
      m_range{0, 100},
      m_line_size(1),
      m_page_size(20),
      m_position(0),
      m_is_dragging(false),
      m_thumb_position(0) {
  m_thumb = new Box(nullptr, nullptr);
  auto thumb_style = StyleSheet();
  thumb_style.get(Any()).set_override(Rule::Override::NONE).
    set(BackgroundColor(QColor("#C8C8C8")));
  m_thumb->set_style(std::move(thumb_style));
  m_track = new Box(m_thumb, this);
  auto track_style = m_track->get_style();
  track_style.get(Any()).set(border(0, QColor(0, 0, 0)));
  if(m_orientation == Qt::Orientation::Vertical) {
    m_thumb->setFixedHeight(scale_height(20));
    m_thumb->setSizePolicy(
      QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    track_style.get(Any()).set(PaddingTop(m_thumb_position));
    track_style.get(Any()).set(PaddingLeft(0));
  } else {
    m_thumb->setFixedWidth(scale_width(20));
    m_thumb->setSizePolicy(QSizePolicy::Policy::Fixed,
      QSizePolicy::Policy::Expanding);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    track_style.get(Any()).set(PaddingTop(0));
    track_style.get(Any()).set(PaddingLeft(m_thumb_position));
  }
  track_style.get(Any()).set(PaddingRight(0));
  track_style.get(Any()).set(PaddingBottom(0));
  m_track->set_style(std::move(track_style));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_track);
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
      auto delta = std::clamp(position - m_drag_position,
        -m_thumb_position, region - m_thumb_position);
      auto updated_thumb_position =
        static_cast<double>(m_thumb_position + delta);
      set_position(static_cast<int>(
        std::ceil((updated_thumb_position * (m_range.m_end - m_range.m_start) +
        region * m_range.m_start) / region)));
      m_drag_position += delta;
    }
  }
  StyledWidget::mouseMoveEvent(event);
}

void ScrollBar::mousePressEvent(QMouseEvent* event) {
  if(m_thumb->rect().contains(m_thumb->mapFromGlobal(event->globalPos()))) {
    m_drag_position = ::get_position(m_orientation, event->windowPos());
    m_is_dragging = true;
  }
  StyledWidget::mousePressEvent(event);
}

void ScrollBar::mouseReleaseEvent(QMouseEvent* event) {
  m_is_dragging = false;
  StyledWidget::mouseReleaseEvent(event);
}

void ScrollBar::resizeEvent(QResizeEvent* event) {
  update_thumb();
}

void ScrollBar::update_thumb() {
  auto region =
    get_size(m_orientation, size()) - get_size(m_orientation, m_thumb->size());
  m_thumb_position =
    region * (m_position - m_range.m_start) / (m_range.m_end - m_range.m_start);
  auto track_style = m_track->get_style();
  if(m_orientation == Qt::Orientation::Vertical) {
    track_style.get(Any()).set(PaddingTop(m_thumb_position));
  } else {
    track_style.get(Any()).set(PaddingLeft(m_thumb_position));
  }
  m_track->set_style(std::move(track_style));
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
