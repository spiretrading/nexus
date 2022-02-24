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
}

SplitView::SplitView(QWidget& primary, QWidget& secondary, QWidget* parent)
    : QWidget(parent),
      m_primary(&primary),
      m_secondary(&secondary),
      m_orientation(Qt::Orientation::Horizontal) {
  match(*m_primary, Primary());
  match(*m_secondary, Secondary());
  m_divider = new Box(nullptr);
  update_style(*m_divider, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
  });
  match(*m_divider, Divider());
  auto layout = new QBoxLayout(to_direction(m_orientation), this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  layout->setAlignment(to_alignment(m_orientation));
  m_divider->setFixedWidth(scale_width(2));
  layout->addWidget(m_primary);
  layout->addWidget(m_divider);
  layout->addWidget(m_secondary);
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
          if(m_orientation == Qt::Orientation::Horizontal) {
            m_divider->setMinimumHeight(0);
            m_divider->setMaximumHeight(QWIDGETSIZE_MAX);
            m_divider->setFixedWidth(scale_width(2));
          } else {
            m_divider->setMinimumWidth(0);
            m_divider->setMaximumWidth(QWIDGETSIZE_MAX);
            m_divider->setFixedHeight(scale_height(2));
          }
          static_cast<QBoxLayout*>(layout())->setDirection(
            to_direction(m_orientation));
          layout()->setAlignment(to_alignment(m_orientation));
        });
      });
  }
}
