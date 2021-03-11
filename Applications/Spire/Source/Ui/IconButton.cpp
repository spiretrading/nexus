#include "Spire/Ui/IconButton.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Tooltip.hpp"

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
  : m_blur_color("#7F5EEC"),
    m_checked_blur_color("#1FD37A"),
    m_checked_color(m_checked_blur_color),
    m_checked_hovered_color("#2CAC79"),
    m_default_color(m_blur_color),
    m_disabled_color("#D0D0D0"),
    m_disabled_background_color("#F5F5F5"),
    m_hover_color("#4B23A0"),
    m_default_background_color(m_disabled_background_color),
    m_hover_background_color("#E3E3E3") {}

IconButton::IconButton(QImage icon, QWidget* parent)
  : IconButton(icon, {}, parent) {}

IconButton::IconButton(QImage icon, Style style, QWidget* parent)
    : QAbstractButton(parent),
      m_icon(std::move(icon)),
      m_style(std::move(style)),
      m_tooltip(nullptr) {
  setAttribute(Qt::WA_Hover);
}

bool IconButton::event(QEvent* event) {
  if(event->type() == QEvent::ToolTipChange) {
    delete_later(m_tooltip);
    if(!toolTip().isEmpty()) {
      m_tooltip = make_text_tooltip(toolTip(), this);
    }
  }
  return QAbstractButton::event(event);
}

void IconButton::keyPressEvent(QKeyEvent* event) {
  if(!event->isAutoRepeat()) {
    switch(event->key()) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
        nextCheckState();
        Q_EMIT clicked(isChecked());
        return;
    }
  }
  QAbstractButton::keyPressEvent(event);
}

void IconButton::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(rect(), get_background_color());
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  image_painter.fillRect(icon.rect(), get_current_icon_color());
  painter.drawPixmap((width() - icon.width()) / 2,
    (height() - icon.height()) / 2, icon);
  if(hasFocus()) {
    draw_border(rect(), "#4B23A0", &painter);
  }
}

QSize IconButton::sizeHint() const {
  return DEFAULT_SIZE();
}

const QColor& IconButton::get_background_color() const {
  if(!isEnabled()) {
    return m_style.m_disabled_background_color;
  } else if(!underMouse()) {
    return m_style.m_default_background_color;
  }
  return m_style.m_hover_background_color;
}

const QColor& IconButton::get_current_icon_color() const {
  if(isEnabled()) {
    if(isChecked()) {
      if(underMouse()) {
        return m_style.m_checked_hovered_color;
      } else if(!window()->isActiveWindow()) {
        return m_style.m_checked_blur_color;
      }
      return m_style.m_checked_color;
    }
    if(underMouse()) {
      return m_style.m_hover_color;
    } else if(!window()->isActiveWindow()) {
      return m_style.m_blur_color;
    }
    return m_style.m_default_color;
  }
  return m_style.m_disabled_color;
}

const IconButton::Style& IconButton::get_style() const {
  return m_style;
}

void IconButton::set_style(const Style& style) {
  m_style = style;
  update();
}
