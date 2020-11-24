#include "Spire/Charting/TrendLineStyleDropDownMenu.hpp"
#include <QLayout>
#include <QPainter>
#include <QPaintEvent>
#include "Spire/Charting/StyleDropDownMenuItem.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

TrendLineStyleDropDownMenu::TrendLineStyleDropDownMenu(
    QWidget* parent)
    : StaticDropDownMenu({}, parent),
      m_dropdown_image(imageFromSvg(":/Icons/arrow-down.svg", scale(6, 4))) {
  m_current_style = TrendLineStyle::SOLID;
  m_selected_connection = connect_value_selected_signal(
    [=] (const auto& value) {
      on_item_selected(value.value<TrendLineStyle>());
    });
  insert_item(new StyleDropDownMenuItem(TrendLineStyle::SOLID, this));
  insert_item(new StyleDropDownMenuItem(TrendLineStyle::SOLID_WIDE, this));
  insert_item(new StyleDropDownMenuItem(TrendLineStyle::DASHED, this));
  insert_item(new StyleDropDownMenuItem(TrendLineStyle::DASHED_WIDE, this));
  insert_item(new StyleDropDownMenuItem(TrendLineStyle::DOTTED, this));
  insert_item(new StyleDropDownMenuItem(TrendLineStyle::DOTTED_WIDE, this));
}

TrendLineStyle TrendLineStyleDropDownMenu::get_style() const {
  return m_current_style;
}

connection TrendLineStyleDropDownMenu::connect_style_signal(
    const StyleSignal::slot_type& slot) const {
  return m_style_signal.connect(slot);
}

void TrendLineStyleDropDownMenu::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus() || underMouse()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
  auto line_y = event->rect().height() / 2;
  draw_trend_line(painter, m_current_style, Qt::black, scale_width(8),
    line_y, scale_width(8) + scale_width(30), line_y);
  painter.drawImage(
    QPoint(width() - (m_dropdown_image.width() + scale_width(8)),
    scale_height(8)), m_dropdown_image);
}

void TrendLineStyleDropDownMenu::on_item_selected(TrendLineStyle style) {
  m_current_style = style;
  m_style_signal(m_current_style);
  update();
}
