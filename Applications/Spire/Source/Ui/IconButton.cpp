#include "Spire/Ui/IconButton.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  auto DEFAULT_SIZE() {
    static auto size = scale(26, 26);
    return size;
  }
}

IconButton::Style::Style()
    : m_default_color("#7F5EEC"),
      m_hover_color("#4B23A0"),
      m_disabled_color("#D0D0D0"),
      m_hover_background_color("#E3E3E3"),
      m_blur_color("#7F5EEC") {
  m_default_background_color.setAlpha(0);
}

IconButton::IconButton(QImage icon, QWidget* parent)
  : IconButton(icon, {}, parent) {}

IconButton::IconButton(QImage icon, Style style, QWidget* parent)
    : QAbstractButton(parent),
      m_icon(std::move(icon)),
      m_style(std::move(style)) {
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

void IconButton::keyPressEvent(QKeyEvent* event) {
  if(!event->isAutoRepeat() && (event->key() == Qt::Key_Enter ||
      event->key() == Qt::Key_Return)) {
    event->accept();
    setDown(true);
    Q_EMIT pressed();
    return;
  }
  QAbstractButton::keyPressEvent(event);
}

void IconButton::keyReleaseEvent(QKeyEvent* event) {
  if(!event->isAutoRepeat() && isDown()) {
    event->accept();
    setDown(false);
    Q_EMIT released();
    Q_EMIT clicked(isChecked());
    return;
  }
  QAbstractButton::keyReleaseEvent(event);
}

void IconButton::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(!underMouse() || !isEnabled()) {
    painter.fillRect(rect(), m_style.m_default_background_color);
  } else {
    painter.fillRect(rect(), m_style.m_hover_background_color);
  }
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  image_painter.fillRect(icon.rect(), get_current_icon_color());
  painter.drawPixmap((width() - icon.width()) / 2,
    (height() - icon.height()) / 2, icon);
  if(hasFocus()) {
    painter.setPen({QColor("#4B23A0"), static_cast<double>(scale_width(1))});
    painter.drawRect(rect().adjusted(0, 0, -scale_width(1), -scale_height(1)));
  }
}

QSize IconButton::sizeHint() const {
  return DEFAULT_SIZE();
}

const QColor& IconButton::get_current_icon_color() const {
  if(isEnabled()) {
    if(underMouse()) {
      return m_style.m_hover_color;
    } else if(!window()->isActiveWindow()) {
      return m_style.m_blur_color;
    }
    return m_style.m_default_color;
  }
  return m_style.m_disabled_color;
}
