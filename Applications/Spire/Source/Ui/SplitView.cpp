#include "Spire/Ui/SplitView.hpp"
#include <QBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
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
}
