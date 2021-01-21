#include "Spire/Ui/ToggleButton.hpp"
#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto BACKGROUND_COLOR = QColor("#F5F5F5");
  const auto CHECKED_COLOR = QColor("#1FD37A");
  const auto CHECKED_HOVER_COLOR = QColor("#2CAC79");
  const auto DISABLED_COLOR = QColor("#D0D0D0");
  const auto ENABLED_COLOR = QColor("#7F5EEC");
  const auto HOVERED_BACKGROUND_COLOR = QColor("#E3E3E3");
  const auto HOVERED_FOCUSED_COLOR = QColor("#4B23A0");

  auto DEFAULT_SIZE() {
    static auto size = scale(26, 26);
    return size;
  }
}

ToggleButton::ToggleButton(QImage icon, QWidget* parent)
    : QAbstractButton(parent),
      m_icon(std::move(icon)) {
  setCheckable(true);
  setAttribute(Qt::WA_Hover);
  setStyleSheet(QString(R"(
    QToolTip {
      background-color: white;
      border: 1px solid #C8C8C8;
      color: black;
      font-family: Roboto;
      font-size: %1px;
      padding: %2px %3px %2px %3px;
    })").arg(scale_height(10)).arg(scale_height(2)).arg(scale_width(6)));
}

void ToggleButton::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(!underMouse() || !isEnabled()) {
    painter.fillRect(rect(), BACKGROUND_COLOR);
  } else {
    painter.fillRect(rect(), HOVERED_BACKGROUND_COLOR);
  }
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  image_painter.fillRect(icon.rect(), get_current_icon_color());
  painter.drawPixmap((width() - icon.width()) / 2,
    (height() - icon.height()) / 2, icon);
  if(hasFocus()) {
    painter.setPen({HOVERED_FOCUSED_COLOR,
      static_cast<double>(scale_width(1))});
    painter.drawRect(rect().adjusted(0, 0, -scale_width(1), -scale_height(1)));
  }
}

QSize ToggleButton::sizeHint() const {
  return DEFAULT_SIZE();
}

const QColor& ToggleButton::get_current_icon_color() const {
  if(isEnabled()) {
    if(isChecked()) {
      if(underMouse()) {
        return CHECKED_HOVER_COLOR;
      }
      return CHECKED_COLOR;
    }
    if(underMouse()) {
      return HOVERED_FOCUSED_COLOR;
    }
    return ENABLED_COLOR;
  }
  return DISABLED_COLOR;
}
