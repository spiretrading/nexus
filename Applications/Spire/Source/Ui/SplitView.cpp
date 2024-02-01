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
  QWidget* m_body;

  Panel(QWidget& body)
      : m_body(&body) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    enclose(*this, *m_body);
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
      m_orientation(Qt::Orientation::Horizontal) {
  match(*m_primary, Primary());
  match(*m_secondary, Secondary());
  m_divider = new DividerBox(*this, m_orientation);
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
    std::bind_front(&SplitView::update_divider_state, this));
  m_primary->stackUnder(m_divider->m_sash);
  m_secondary->m_layout_signal.connect(
    std::bind_front(&SplitView::update_divider_state, this));
  m_secondary->stackUnder(m_divider->m_sash);
  update_divider_state();
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(Qt::Orientation::Horizontal);
    style.get(
        Any() > (is_a<Sash>() && (Hover() || Drag())) % is_a<DividerBox>()).
      set(BackgroundColor(QColor(0x4B23A0)));
  });
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&SplitView::on_style, this));
}

void SplitView::update_divider_state() {
  auto disabled = m_orientation == Qt::Orientation::Horizontal && (
    m_primary->m_body->minimumWidth() == m_primary->m_body->maximumWidth() ||
    m_secondary->m_body->minimumWidth() ==
    m_secondary->m_body->maximumWidth()) ||
    m_orientation == Qt::Orientation::Vertical && (
    m_primary->m_body->minimumHeight() ==
    m_primary->m_body->maximumHeight() ||
    m_secondary->m_body->minimumHeight() ==
    m_secondary->m_body->maximumHeight());
  m_divider->setDisabled(disabled);
  m_divider->m_sash->setDisabled(disabled);
}

void SplitView::on_drag(int offset) {
  auto set_fixed_size = [&] (auto set_fixed_size, int cursor, int start,
      int end, int primary_hint, int secondary_hint, int primary_minimum,
      int primary_maximum, int secondary_minimum, int secondary_maximum,
      int size, int body_size, int divider_size) {
    if(cursor <= start) {
      if(find_focus_state(*m_primary) != FocusObserver::State::NONE) {
        setFocus(Qt::FocusReason::OtherFocusReason);
      }
      m_primary->hide();
    } else if(cursor >= end) {
      if(find_focus_state(*m_secondary) != FocusObserver::State::NONE) {
        setFocus(Qt::FocusReason::OtherFocusReason);
      }
      m_secondary->hide();
      (m_primary->*set_fixed_size)(QWIDGETSIZE_MAX);
    } else if(m_secondary->isHidden() && cursor < end) {
      (m_primary->*set_fixed_size)(size - divider_size -
        std::max(secondary_hint, secondary_minimum));
      m_secondary->show();
    } else {
      auto primary_size = [&] {
        if(offset < 0) {
          return std::max({primary_hint, primary_minimum, size + offset,
            body_size - divider_size - secondary_maximum});
        }
        auto base = body_size - std::max(secondary_hint, secondary_minimum);
        return std::min({primary_maximum, size + offset, base - divider_size});
      }();
      (m_primary->*set_fixed_size)(primary_size);
      if(m_primary->isHidden() && cursor > start) {
        m_primary->show();
      }
    }
  };
  if(m_orientation == Qt::Orientation::Horizontal) {
    auto cursor = QCursor::pos().x();
    auto left = mapToGlobal(QPoint(0, 0)).x();
    auto right = mapToGlobal(QPoint(width() - 1, 0)).x();
    set_fixed_size(&QWidget::setFixedWidth, cursor, left, right,
      m_primary->m_body->sizeHint().width(),
      m_secondary->m_body->sizeHint().width(),
      m_primary->m_body->minimumWidth(), m_primary->m_body->maximumWidth(),
      m_secondary->m_body->minimumWidth(), m_secondary->m_body->maximumWidth(),
      m_primary->width(), width(), m_divider->width());
  } else {
    auto cursor = QCursor::pos().y();
    auto top = mapToGlobal(QPoint(0, 0)).y();
    auto bottom = mapToGlobal(QPoint(0, height() - 1)).y();
    set_fixed_size(&QWidget::setFixedHeight, cursor, top, bottom,
      m_primary->m_body->sizeHint().height(),
      m_secondary->m_body->sizeHint().height(),
      m_primary->m_body->minimumHeight(), m_primary->m_body->maximumHeight(),
      m_secondary->m_body->minimumHeight(),
      m_secondary->m_body->maximumHeight(), m_primary->height(), height(),
      m_divider->height());
  }
}

void SplitView::on_reset() {
  if(m_orientation == Qt::Orientation::Horizontal) {
    m_primary->setFixedWidth(std::max(m_primary->m_body->minimumWidth(),
      (width() - DividerBox::base_size) / 2));
  } else {
    m_primary->setFixedHeight(std::max(m_primary->m_body->minimumHeight(),
      (height() - DividerBox::base_size) / 2));
  }
  m_primary->show();
  m_secondary->show();
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
        update_divider_state();
        static_cast<QBoxLayout*>(layout())->setDirection(
          to_direction(m_orientation));
        layout()->setAlignment(to_alignment(m_orientation));
      });
    });
  }
}
