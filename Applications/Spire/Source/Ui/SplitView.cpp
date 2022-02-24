#include "Spire/Ui/SplitView.hpp"
#include <QBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"

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
    static const auto base_size = 12;
    using QWidget::QWidget;

    void set_orientation(Qt::Orientation orientation) {
      if(orientation == Qt::Orientation::Horizontal) {
        setCursor(Qt::SizeHorCursor);
      } else {
        setCursor(Qt::SizeVerCursor);
      }
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
    QWidget::moveEvent(event);
  }

  void resizeEvent(QResizeEvent* event) override {
    if(m_orientation == Qt::Orientation::Horizontal) {
      m_sash->resize(event->size().width() +
        scale_width(Sash::base_size - base_size), event->size().height());
    } else {
      m_sash->resize(event->size().width(),
        event->size().height() + scale_height(Sash::base_size - base_size));
    }
    QWidget::resizeEvent(event);
  }
};

SplitView::SplitView(QWidget& primary, QWidget& secondary, QWidget* parent)
    : QWidget(parent),
      m_primary(&primary),
      m_secondary(&secondary),
      m_orientation(Qt::Orientation::Horizontal) {
  match(*m_primary, Primary());
  match(*m_secondary, Secondary());
  m_divider = new DividerBox(*this, m_orientation);
  match(*m_divider, Divider());
  update_style(*m_divider, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
  });
  auto layout = new QBoxLayout(to_direction(m_orientation), this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  layout->setAlignment(to_alignment(m_orientation));
  layout->addWidget(m_primary);
  layout->addWidget(m_divider);
  layout->addWidget(m_secondary);
  m_primary->stackUnder(m_divider->m_sash);
  m_secondary->stackUnder(m_divider->m_sash);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(Qt::Orientation::Horizontal);
  });
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&SplitView::on_style, this));
}

void SplitView::on_style() {
  auto& stylist = find_stylist(*this);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (EnumProperty<Qt::Orientation> orientation) {
        stylist.evaluate(orientation, [=] (auto orientation) {
          if(orientation == m_orientation) {
            return;
          }
          m_orientation = orientation;
          m_divider->set_orientation(m_orientation);
          static_cast<QBoxLayout*>(layout())->setDirection(
            to_direction(m_orientation));
          layout()->setAlignment(to_alignment(m_orientation));
        });
      });
  }
}
