#include "Spire/Ui/DropdownMenuItem.hpp"
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto LEFT_PADDING() {
    static auto padding = scale_width(5);
    return padding;
  }

  auto RIGHT_PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

DropDownMenuItem::DropDownMenuItem(const QString& text, QWidget* parent)
    : QLabel(text, parent),
      m_is_highlighted(false) {
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  setFixedHeight(scale_height(20));
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  setFont(font);
}

void DropDownMenuItem::set_highlight() {
  m_is_highlighted = true;
}

void DropDownMenuItem::remove_highlight() {
  m_is_highlighted = false;
}

connection DropDownMenuItem::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void DropDownMenuItem::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    m_selected_signal(text());
  }
}

void DropDownMenuItem::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_selected_signal(text());
  }
}

void DropDownMenuItem::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(m_is_highlighted || underMouse()) {
    painter.fillRect(event->rect(), QColor("#F2F2FF"));
  } else {
    painter.fillRect(event->rect(), Qt::white);
  }
  auto metrics = QFontMetrics(font());
  auto shortened_text = metrics.elidedText(text(), Qt::ElideRight,
    width() - RIGHT_PADDING());
  painter.drawText(LEFT_PADDING(), metrics.height(), shortened_text);
}
