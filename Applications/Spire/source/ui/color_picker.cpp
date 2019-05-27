#include "spire/ui/color_picker.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include "spire/spire/dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

ColorPicker::ColorPicker(int picker_width, int picker_height, QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setObjectName("color_picker");
  setStyleSheet(QString(R"(
    #color_picker {
      background-color: #FFFFFF;
      border: 1px solid #A0A0A0;
      border-top: none;
    })"));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(2, 1, 2, 2);
  auto label = new QLabel(this);
  layout->addWidget(label);
  m_gradient = QImage(picker_width, picker_height, QImage::Format_RGB32);
  for (auto i = 0; i < m_gradient.height(); ++i) {
    auto line = (QRgb *) m_gradient.scanLine(i);
    for (int j = 0; j < m_gradient.width(); ++j) {
      line[j] = QColor(0, 0, 0).rgb();
    }
  }
  label->setPixmap(QPixmap::fromImage(m_gradient));
}

connection ColorPicker::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}
