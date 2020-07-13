#include "Spire/Ui/DropDownItem.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

DropDownItem::DropDownItem(const QVariant& value,
    QWidget* parent)
    : QWidget(parent),
      m_value(value),
      m_is_highlighted(false) {
  setAttribute(Qt::WA_Hover);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  setFont(font);
}

void DropDownItem::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_selected_signal(m_value);
  }
}

void DropDownItem::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(underMouse() || m_is_highlighted) {
    painter.fillRect(rect(), QColor("#F2F2FF"));
  } else {
    painter.fillRect(rect(), Qt::white);
  }
  // TODO: move to anon namespace
  auto LEFT_PADDING = []{ return scale_width(8); };
  auto RIGHT_PADDING = []{ return scale_width(12); };
  auto metrics = QFontMetrics(font());
  auto shortened_text = metrics.elidedText(
    m_item_delegate.displayText(m_value), Qt::ElideRight,
    width() - RIGHT_PADDING());
  painter.drawText(LEFT_PADDING(), metrics.height(), shortened_text);
}

connection DropDownItem::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

const QVariant& DropDownItem::get_value() const {
  return m_value;
}

void DropDownItem::set_highlight() {
  m_is_highlighted = true;
  update();
}

void DropDownItem::reset_highlight() {
  m_is_highlighted = false;
  update();
}
