#include "Spire/Charting/ColorPicker.hpp"
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

ColorPicker::ColorPicker(int picker_width, int picker_height, QWidget* parent)
    : QWidget(parent) {
  setObjectName("color_picker");
  setStyleSheet(QString(R"(
    #color_picker {
      background-color: #FFFFFF;
      border: 1px solid #A0A0A0;
      border-top: none;
    })"));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(2, 1, 2, 2);
  m_gradient_label = new QLabel(this);
  m_gradient_label->setMouseTracking(true);
  layout->addWidget(m_gradient_label);
  auto gradient = QImage(":/Icons/color-picker-display.png").scaled(
    picker_width, picker_height);
  m_gradient_label->setPixmap(QPixmap::fromImage(gradient));
  m_gradient_label->installEventFilter(this);
  auto cursor = QCursor(QPixmap::fromImage(
    imageFromSvg(":/Icons/color-picker-cursor.svg", scale(10, 10))));
  setCursor(cursor);
}

connection ColorPicker::connect_preview_signal(
    const ColorSignal::slot_type& slot) const {
  return m_preview_signal.connect(slot);
}

connection ColorPicker::connect_selected_signal(
    const ColorSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

bool ColorPicker::eventFilter(QObject* watched, QEvent* event) {
  if(isVisible()) {
    if(event->type() == QEvent::MouseButtonPress) {
      auto e = static_cast<QMouseEvent*>(event);
      if(e->button() == Qt::LeftButton) {
        m_selected_signal(gradient_color_at(e->pos()));
      }
    } else if(event->type() == QEvent::MouseMove) {
      auto e = static_cast<QMouseEvent*>(event);
      m_preview_signal(gradient_color_at(e->pos()));
    }
  }
  return QWidget::eventFilter(watched, event);
}

QColor ColorPicker::gradient_color_at(const QPoint& pos) {
  auto pixmap = m_gradient_label->grab({pos, QSize(1, 1)});
  return pixmap.toImage().pixelColor(0, 0);
}
