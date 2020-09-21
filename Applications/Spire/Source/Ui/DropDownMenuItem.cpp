#include "Spire/Ui/DropDownMenuItem.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto LEFT_PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
  
  auto RIGHT_PADDING() {
    static auto padding = scale_width(12);
    return padding;
  }
}

DropDownMenuItem::DropDownMenuItem(QVariant value, QWidget* parent)
  : DropDownMenuItem(std::move(value), {}, parent) {}

DropDownMenuItem::DropDownMenuItem(QVariant value, QImage icon,
    QWidget* parent)
    : QWidget(parent),
      m_value(std::move(value)),
      m_icon(std::move(icon)),
      m_is_highlighted(false) {
  setAttribute(Qt::WA_Hover);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  setFont(font);
}

void DropDownMenuItem::enterEvent(QEvent* event) {
  auto value = m_value;
  m_highlighted_signal(value);
}

void DropDownMenuItem::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    auto value = m_value;
    m_selected_signal(value);
  }
}

void DropDownMenuItem::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(underMouse() || m_is_highlighted) {
    painter.fillRect(rect(), QColor("#F2F2FF"));
  } else {
    painter.fillRect(rect(), Qt::white);
  }
  auto metrics = QFontMetrics(font());
  auto shortened_text = metrics.elidedText(
    m_item_delegate.displayText(m_value), Qt::ElideRight,
    width() - RIGHT_PADDING());
  if(!m_icon.isNull()) {
    painter.drawImage(LEFT_PADDING(), (height() / 2) - (m_icon.height() / 2),
      m_icon);
    painter.drawText(2 * LEFT_PADDING() + m_icon.width(), metrics.height(),
      shortened_text);
  } else {
    painter.drawText(LEFT_PADDING(), metrics.height(), shortened_text);
  }
}

connection DropDownMenuItem::connect_highlighted_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

connection DropDownMenuItem::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

const QVariant& DropDownMenuItem::get_value() const {
  return m_value;
}

void DropDownMenuItem::set_highlight() {
  m_is_highlighted = true;
  update();
}

void DropDownMenuItem::reset_highlight() {
  m_is_highlighted = false;
  update();
}
