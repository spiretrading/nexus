#include "Spire/Ui/SplitView.hpp"
#include <boost/optional/optional.hpp>
#include <QBoxLayout>
#include <QMoveEvent>
#include <QResizeEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  QBoxLayout::Direction to_direction(Qt::Orientation orientation) {
    if(orientation == Qt::Orientation::Horizontal) {
      return QBoxLayout::Direction::LeftToRight;
    }
    return QBoxLayout::Direction::TopToBottom;
  }

  Qt::Alignment to_alignment(Qt::Orientation orientation) {
    if(orientation == Qt::Orientation::Horizontal) {
      return Qt::AlignLeft;
    }
    return Qt::AlignTop;
  }

  int get_size(const QSize& size, Qt::Orientation orientation) {
    if(orientation == Qt::Orientation::Horizontal) {
      return size.width();
    }
    return size.height();
  }

  int get_position(const QPoint& point, Qt::Orientation orientation) {
    if(orientation == Qt::Orientation::Horizontal) {
      return point.x();
    }
    return point.y();
  }

  struct Sash : QWidget {
    using DragSignal = Signal<void (int offset)>;
    using ResetSignal = Signal<void ()>;
    static const auto base_size = 4;
    mutable DragSignal m_drag_signal;
    mutable ResetSignal m_reset_signal;
    Qt::Orientation m_orientation;
    int m_last_mouse_position;
    optional<int> m_drag_origin;

    using QWidget::QWidget;

    void set_orientation(Qt::Orientation orientation) {
      m_orientation = orientation;
      if(m_orientation == Qt::Orientation::Horizontal) {
        setCursor(Qt::SizeHorCursor);
      } else {
        setCursor(Qt::SizeVerCursor);
      }
    }

    void changeEvent(QEvent* event) override {
      if(isEnabled()) {
        if(m_orientation == Qt::Orientation::Horizontal) {
          setCursor(Qt::SizeHorCursor);
        } else {
          setCursor(Qt::SizeVerCursor);
        }
      } else {
        setCursor(QCursor());
      }
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override {
      if(event->button() == Qt::LeftButton) {
        m_reset_signal();
        return;
      }
      QWidget::mouseDoubleClickEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override {
      if(!m_drag_origin) {
        return QWidget::mouseMoveEvent(event);
      }
      if(m_orientation == Qt::Orientation::Horizontal) {
        if(event->globalPos().x() - m_last_mouse_position < 0 &&
            event->pos().x() < *m_drag_origin ||
            event->globalPos().x() - m_last_mouse_position > 0 &&
            event->pos().x() > *m_drag_origin) {
          m_drag_signal(event->globalPos().x() - m_last_mouse_position);
        }
        m_last_mouse_position = event->globalPos().x();
      } else {
        if(event->globalPos().y() - m_last_mouse_position < 0 &&
            event->pos().y() < *m_drag_origin ||
            event->globalPos().y() - m_last_mouse_position > 0 &&
            event->pos().y() > *m_drag_origin) {
          m_drag_signal(event->globalPos().y() - m_last_mouse_position);
        }
        m_last_mouse_position = event->globalPos().y();
      }
    }

    void mousePressEvent(QMouseEvent* event) override {
      if(event->button() == Qt::LeftButton && !m_drag_origin) {
        if(m_orientation == Qt::Orientation::Horizontal) {
          m_last_mouse_position = event->globalPos().x();
          m_drag_origin = event->pos().x();
        } else {
          m_last_mouse_position = event->globalPos().y();
          m_drag_origin = event->pos().y();
        }
        match(*this, Drag());
        return;
      }
      QWidget::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
      if(event->button() == Qt::LeftButton && m_drag_origin) {
        m_drag_origin = none;
        unmatch(*this, Drag());
        return;
      }
      QWidget::mouseReleaseEvent(event);
    }
  };
}

struct SplitView::DividerBox : Box {
  static const auto base_size = 2;
  Sash* m_sash;
  Qt::Orientation m_orientation;

  DividerBox(QWidget& parent, Qt::Orientation orientation)
      : Box(nullptr, &parent),
        m_sash(new Sash(&parent)) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if(orientation == Qt::Orientation::Horizontal) {
      m_orientation = Qt::Orientation::Vertical;
    } else {
      m_orientation = Qt::Orientation::Horizontal;
    }
    set_orientation(orientation);
    stackUnder(m_sash);
  }

  void set_orientation(Qt::Orientation orientation) {
    if(orientation == m_orientation) {
      return;
    }
    m_orientation = orientation;
    if(m_orientation == Qt::Orientation::Horizontal) {
      setMinimumHeight(0);
      setMaximumHeight(QWIDGETSIZE_MAX);
      setFixedWidth(scale_width(base_size));
    } else {
      setMinimumWidth(0);
      setMaximumWidth(QWIDGETSIZE_MAX);
      setFixedHeight(scale_height(base_size));
    }
    m_sash->set_orientation(m_orientation);
  }

  void moveEvent(QMoveEvent* event) override {
    if(m_orientation == Qt::Orientation::Horizontal) {
      m_sash->move(
        event->pos().x() - scale_width((Sash::base_size - base_size) / 2), 0);
    } else {
      m_sash->move(
        0, event->pos().y() - scale_height((Sash::base_size - base_size) / 2));
    }
    Box::moveEvent(event);
  }

  void resizeEvent(QResizeEvent* event) override {
    if(m_orientation == Qt::Orientation::Horizontal) {
      m_sash->resize(event->size().width() +
        scale_width(Sash::base_size - base_size), event->size().height());
    } else {
      m_sash->resize(event->size().width(),
        event->size().height() + scale_height(Sash::base_size - base_size));
    }
    Box::resizeEvent(event);
  }
};

struct SplitView::Panel : QWidget {
  using LayoutSignal = Signal<void ()>;
  mutable LayoutSignal m_layout_signal;

  Panel(QWidget& body) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    enclose(*this, body);
  }

  bool event(QEvent* event) override {
    if(event->type() == QEvent::LayoutRequest) {
      m_layout_signal();
    }
    return QWidget::event(event);
  }
};

SplitView::SplitView(QWidget& primary, QWidget& secondary, QWidget* parent)
    : QWidget(parent),
      m_primary(new Panel(primary)),
      m_secondary(new Panel(secondary)),
      m_orientation(Qt::Orientation::Horizontal),
      m_divider(new DividerBox(*this, m_orientation)),
      m_primary_ratio(0.5) {
  match(*m_primary, Primary());
  match(*m_secondary, Secondary());
  match(*m_divider, Divider());
  update_style(*m_divider, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
  });
  m_divider->m_sash->m_drag_signal.connect(
    std::bind_front(&SplitView::on_drag, this));
  m_divider->m_sash->m_reset_signal.connect(
    std::bind_front(&SplitView::on_reset, this));
  auto layout = new QBoxLayout(to_direction(m_orientation), this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  layout->setAlignment(to_alignment(m_orientation));
  layout->addWidget(m_primary);
  layout->addWidget(m_divider);
  layout->addWidget(m_secondary);
  m_primary->m_layout_signal.connect(
    std::bind_front(&SplitView::update_split, this));
  m_primary->stackUnder(m_divider->m_sash);
  m_secondary->m_layout_signal.connect(
    std::bind_front(&SplitView::update_split, this));
  m_secondary->stackUnder(m_divider->m_sash);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(Qt::Orientation::Horizontal);
    style.get(Any() >
        (is_a<Sash>() && (Hover() || Drag())) % is_a<DividerBox>()).
      set(BackgroundColor(QColor(0x4B23A0)));
  });
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&SplitView::on_style, this));
}

QSize SplitView::minimumSizeHint() const {
  auto primary = m_primary->minimumSizeHint();
  auto secondary = m_secondary->minimumSizeHint();
  if(m_orientation == Qt::Orientation::Horizontal) {
    return QSize(primary.width() + m_divider->width() + secondary.width(),
      std::max(primary.height(), secondary.height()));
  }
  return QSize(std::max(primary.width(), secondary.width()),
    primary.height() + m_divider->height() + secondary.height());
}

void SplitView::resizeEvent(QResizeEvent* event) {
  update_split();
  QWidget::resizeEvent(event);
}

int SplitView::get_content_size() const {
  return std::max(get_size(size(), m_orientation) -
    get_size(m_divider->size(), m_orientation), 0);
}

int SplitView::get_primary_size() const {
  if(m_primary->isHidden()) {
    return 0;
  }
  return get_size(m_primary->size(), m_orientation);
}

int SplitView::get_primary_min_expanded_size() const {
  return get_size(m_primary->minimumSizeHint(), m_orientation);
}

int SplitView::get_secondary_min_expanded_size() const {
  return get_size(m_secondary->minimumSizeHint(), m_orientation);
}

int SplitView::get_secondary_reserved_size() const {
  if(m_secondary->isHidden()) {
    return 0;
  }
  return get_secondary_min_expanded_size();
}

int SplitView::get_primary_max_size() const {
  return std::max(get_content_size() - get_secondary_reserved_size(), 0);
}

int SplitView::clamp_primary_size(int size) const {
  return std::max(
    std::min(size, get_primary_max_size()), get_primary_min_expanded_size());
}

void SplitView::resize_primary(int size) {
  if(m_orientation == Qt::Orientation::Horizontal) {
    m_primary->setFixedWidth(size);
  } else {
    m_primary->setFixedHeight(size);
  }
}

void SplitView::set_primary_size(int size) {
  resize_primary(size);
  if(auto content_size = get_content_size(); content_size > 0) {
    m_primary_ratio = static_cast<double>(size) / content_size;
  }
}

void SplitView::update_split() {
  auto content_size = get_content_size();
  if(content_size == 0) {
    return;
  }
  if(m_primary->isHidden()) {
    resize_primary(0);
  } else if(m_secondary->isHidden()) {
    resize_primary(content_size);
  } else {
    resize_primary(
      clamp_primary_size(static_cast<int>(m_primary_ratio * content_size)));
  }
}

void SplitView::on_drag(int offset) {
  auto cursor = get_position(QCursor::pos(), m_orientation);
  auto start = get_position(mapToGlobal(QPoint(0, 0)), m_orientation);
  auto end = start + get_content_size();
  if(cursor <= start) {
    if(find_focus_state(*m_primary) != FocusObserver::State::NONE) {
      setFocus(Qt::FocusReason::OtherFocusReason);
    }
    m_secondary->show();
    set_primary_size(0);
    m_primary->hide();
  } else if(cursor >= end) {
    if(find_focus_state(*m_secondary) != FocusObserver::State::NONE) {
      setFocus(Qt::FocusReason::OtherFocusReason);
    }
    m_secondary->hide();
    m_primary->show();
    set_primary_size(get_content_size());
  } else if(m_secondary->isHidden()) {
    m_secondary->show();
    m_primary->show();
    set_primary_size(get_primary_max_size());
  } else {
    m_primary->show();
    set_primary_size(clamp_primary_size(get_primary_size() + offset));
  }
}

void SplitView::on_reset() {
  m_primary->show();
  m_secondary->show();
  auto available_size = std::max(get_content_size() -
    get_primary_min_expanded_size() - get_secondary_min_expanded_size(), 0);
  set_primary_size(get_primary_min_expanded_size() + available_size / 2);
}

void SplitView::on_style() {
  auto& stylist = find_stylist(*this);
  for(auto& property : stylist.get_computed_block()) {
    property.visit([&] (EnumProperty<Qt::Orientation> orientation) {
      stylist.evaluate(orientation, [=] (auto orientation) {
        if(orientation == m_orientation) {
          return;
        }
        m_orientation = orientation;
        m_divider->set_orientation(m_orientation);
        static_cast<QBoxLayout*>(layout())->setDirection(
          to_direction(m_orientation));
        layout()->setAlignment(to_alignment(m_orientation));
        m_primary->setMinimumSize(0, 0);
        m_primary->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        on_reset();
      });
    });
  }
}
