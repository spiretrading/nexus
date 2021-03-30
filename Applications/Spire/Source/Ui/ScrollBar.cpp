#include "Spire/Ui/ScrollBar.hpp"
#include <QHBoxLayout>
#include <QMetaEnum>
#include "Spire/Ui/Box.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto SCROLL_BAR_SIZE = 13;

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF)));
    style.get(ScrollBarThumb()).
      set(BackgroundColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

ScrollBar::ScrollBar(Qt::Orientation orientation, QWidget* parent)
    : StyledWidget(parent) {
  if(orientation == Qt::Vertical) {
    m_thumb_min_size = scale_height(50);
  } else {
    m_thumb_min_size = scale_width(50);
  }
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_scroll_bar = new QScrollBar(orientation, this);
  m_scroll_bar->setContextMenuPolicy(Qt::NoContextMenu);
  m_scroll_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_scroll_bar);
  set_style(DEFAULT_STYLE());
  connect(m_scroll_bar, &QScrollBar::valueChanged, [=] (auto value) {
    m_position_signal(value);
  });
}

Qt::Orientation ScrollBar::get_orientation() {
  return m_scroll_bar->orientation();
}

ScrollBar::Range ScrollBar::get_range() const {
  return {m_scroll_bar->minimum(), m_scroll_bar->maximum()};
}

void ScrollBar::set_range(const Range& range) {
  m_scroll_bar->setRange(range.m_start, range.m_end);
}

void ScrollBar::set_range(int start, int end) {
  m_scroll_bar->setRange(start, end);
}

int ScrollBar::get_line_size() const {
  return m_scroll_bar->singleStep();
}

void ScrollBar::set_line_size(int size) {
  m_scroll_bar->setSingleStep(size);
}

int ScrollBar::get_page_size() const {
  return m_scroll_bar->pageStep();
}

void ScrollBar::set_page_size(int size) {
  m_scroll_bar->setPageStep(size);
}

int ScrollBar::get_position() const {
  return m_scroll_bar->value();
}

void ScrollBar::set_position(int position) {
  m_scroll_bar->setValue(position);
}

int ScrollBar::get_thumb_min_size() const {
  return m_thumb_min_size;
}

void ScrollBar::set_thumb_min_size(int size) {
  m_thumb_min_size = size;
  style_updated();
}

connection ScrollBar::connect_position_signal(
    const PositionSignal::slot_type& slot) const {
  return m_position_signal.connect(slot);
}

void ScrollBar::selector_updated() {
  auto scroll_bar_computed_style = compute_style();
  auto thumb_computed_style = compute_style(ScrollBarThumb());
  auto orientation = QMetaEnum::fromType<Qt::Orientation>().valueToKey(
    get_orientation());
  auto scroll_bar_style = QString(R"(
    QScrollBar:%1 {
      padding: 0px;)").arg(orientation);
  auto thumb_style = QString("QScrollBar::handle:%1 {").arg(orientation);
  for(auto& property : scroll_bar_computed_style.get_properties()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        scroll_bar_style += "background: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      });
  }
  scroll_bar_style += "}";
  for(auto& property : thumb_computed_style.get_properties()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        thumb_style += "background: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      });
  }
  auto thumb_min_size_style = [=] {
    if(get_orientation() == Qt::Vertical) {
      return QString("min-height: %1px;").arg(m_thumb_min_size);
    } else {
      return QString("min-width: %1px;").arg(m_thumb_min_size);
    }
  }();
  thumb_style += thumb_min_size_style + "}";
  auto style = scroll_bar_style + thumb_style;
  style += QString(R"(
    QScrollBar::add-page:%1, QScrollBar::sub-page:%1 {
      background: none;
    }
    QScrollBar::add-line:%1, QScrollBar::sub-line:%1 {
      width: 0px;
      height: 0px;
    })").arg(orientation);
  setStyleSheet(style);
  StyledWidget::selector_updated();
}
