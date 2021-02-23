#include "Spire/Ui/Box.hpp"
#include <QEvent>
#include <QLayout>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace Spire;

namespace {
  auto create_style_sheet(const QString& state, const Box::Style& style) {
    auto default_sheet = QString(R"(
      #Box%1 {
        background-color: %2;
        border-color: %3;
        margin: 0px;
        outline: none;
      )").arg(state).arg(style.m_background_color.name(QColor::HexArgb)).
        arg(style.m_border_color.name(QColor::HexArgb));
    auto border_sheet = QString();
    if(style.m_borders) {
      border_sheet = QString(R"(
        border-bottom-width: %1px;
        border-left-width: %2px;
        border-right-width: %3px;
        border-style: solid;
        border-top-width: %4px;
      )").arg(style.m_borders->bottom()).arg(style.m_borders->left()).
        arg(style.m_borders->right()).arg(style.m_borders->top());
    }
    auto closing_bracket = QString("}");
    auto style_sheet = QString();
    style_sheet.reserve(default_sheet.length() + border_sheet.length() +
      closing_bracket.length());
    return style_sheet.append(default_sheet).append(border_sheet).
      append(closing_bracket);
  }
}

Box::Box(QWidget* parent)
    : QWidget(parent),
      m_is_hover(false) {
  setObjectName("Box");
  m_style.m_borders = {{scale_width(1), scale_height(1), scale_width(1),
    scale_height(1)}};
  m_style.m_paddings = {{0, 0, 0, 0}};
  m_style.m_background_color = QColor("#FFFFFF");
  m_style.m_border_color = QColor("#C8C8C8");
  m_hover_style = m_style;
  m_focus_style = m_style;
  m_disabled_style = m_style;
  update_style();
}

const Box::Style& Box::get_style() const {
  return m_style;
}

void Box::set_style(const Style& style) {
  m_style = style;
  process_style(style);
  update_style();
}

const Box::Style& Box::get_hover_style() const {
  return m_hover_style;
}

void Box::set_hover_style(const Style& hover_style) {
  m_hover_style = hover_style;
  process_hover_style(hover_style);
  update_style();
}

const Box::Style& Box::get_focus_style() const {
  return m_focus_style;
}

void Box::set_focus_style(const Style& focus_style) {
  m_focus_style = focus_style;
  process_focus_style(focus_style);
  update_style();
}

const Box::Style& Box::get_disabled_style() const {
  return m_disabled_style;
}

void Box::set_disabled_style(const Style& disabled_style) {
  m_disabled_style = disabled_style;
  process_disabled_style(disabled_style);
  update_style();
}

void Box::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_box();
  }
  QWidget::changeEvent(event);
}

bool Box::event(QEvent* event) {
  if(isEnabled()) {
    switch(event->type()) {
    case QEvent::HoverEnter:
      m_is_hover = true;
      update_box();
      break;
    case QEvent::HoverLeave:
      m_is_hover = false;
      update_box();
      break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
      update_box();
      break;
    }
  }
  return QWidget::event(event);
}

QSize Box::sizeHint() const {
  auto size = get_size(m_style);
  if(size.isValid()) {
    return size;
  }
  return scale(180, 26);
}

void Box::process_style(const Style& style) {}

void Box::process_hover_style(const Style& hover_style) {}

void Box::process_focus_style(const Style& focus_style) {}

void Box::process_disabled_style(const Style& disabled_style) {}

QSize Box::get_size(const Style& style) const {
  if(style.m_size) {
    if(auto size = get<QSize>(style.m_size.get_ptr())) {
      return *size;
    } else if(auto relative_size = get<QSizeF>(style.m_size.get_ptr())) {
      if(relative_size->isValid()) {
        auto parent_size = QSize();
        if(auto parent_widget = parentWidget()) {
          if(auto parent_layout = parent_widget->layout()) {
            parent_size = parent_layout->geometry().size();
          } else {
            parent_size = parent_widget->size();
          }
          return QSize({qRound(parent_size.width() * relative_size->width()),
            qRound(parent_size.height() * relative_size->height())});
        }
      }
    }
  }
  return QSize(-1, -1);
}

void Box::resize_box(const Style& style) {
  auto new_size = get_size(style);
  if(new_size.isValid() && !new_size.isNull()) {
    if(size() != new_size) {
      setFixedSize(new_size);
    }
  } else {
    if(m_style.m_size) {
      resize_box(m_style);
    }
  }
}

void Box::update_box() {
  if(isEnabled()) {
    if(m_is_hover) {
      resize_box(m_hover_style);
    } else if(hasFocus()) {
      resize_box(m_focus_style);
    } else {
      resize_box(m_style);
    }
  } else {
    resize_box(m_disabled_style);
  }
}

void Box::update_style() {
  auto default_style_sheet = create_style_sheet("", get_style());
  auto hover_style_sheet = create_style_sheet(":hover", get_hover_style());
  auto focus_hover_style_sheet = create_style_sheet(":focus:hover",
    get_hover_style());
  auto focus_style_sheet = create_style_sheet(":focus", get_focus_style());
  auto disabled_style_sheet = create_style_sheet(":disabled",
    get_disabled_style());
  auto style_sheet = QString();
  style_sheet.reserve(default_style_sheet.length() +
    hover_style_sheet.length() + focus_hover_style_sheet.length() +
    focus_style_sheet.length() + disabled_style_sheet.length());
  setStyleSheet(style_sheet.append(default_style_sheet).
    append(hover_style_sheet).append(focus_hover_style_sheet).
    append(focus_style_sheet).append(disabled_style_sheet));
  update_box();
}
