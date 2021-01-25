#include "Spire/Ui/FlatButton.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

FlatButton::FlatButton(QWidget* parent)
  : FlatButton("", parent) {}

FlatButton::FlatButton(const QString& label, QWidget* parent)
    : QPushButton(label, parent) {
  setObjectName("flat_button");
  setFlat(true);
  setFocusPolicy(Qt::StrongFocus);
  auto button_font = QFont("Roboto");
  button_font.setPixelSize(scale_height(12));
  button_font.setWeight(QFont::Normal);
  button_font.setLetterSpacing(QFont::AbsoluteSpacing, 0);
  setFont(button_font);
  set_style({QColor("#EBEBEB"), QColor("#EBEBEB"), QColor("#000000")},
    {QColor("#4B23A0"), QColor("#4B23A0"), QColor("#FFFFFF")},
    {QColor("#EBEBEB"), QColor("#4B23A0"), QColor("#000000")},
    {QColor("#EBEBEB"), QColor("#EBEBEB"), QColor("#A0A0A0")});
}

void FlatButton::set_style(const Style& default_style, const Style& hover_style,
    const Style& focus_style, const Style& disabled_style) {
  m_default_style = default_style;
  m_hover_style = hover_style;
  m_focus_style = focus_style;
  m_disabled_style = disabled_style;
  update_style();
}

const FlatButton::Style& FlatButton::get_default_style() const {
  return m_default_style;
}

void FlatButton::set_default_style(const Style& default_style) {
  m_default_style = default_style;
  update_style();
}

const FlatButton::Style& FlatButton::get_hover_style() const {
  return m_hover_style;
}

void FlatButton::set_hover_style(const Style& hover_style) {
  m_hover_style = hover_style;
  update_style();
}

const FlatButton::Style& FlatButton::get_focus_style() const {
  return m_focus_style;
}

void FlatButton::set_focus_style(const Style& focus_style) {
  m_focus_style = focus_style;
  update_style();
}

const FlatButton::Style& FlatButton::get_disabled_style() const {
  return m_disabled_style;
}

void FlatButton::set_disabled_style(const Style& disabled_style) {
  m_disabled_style = disabled_style;
  update_style();
}

void FlatButton::set_font_properties(int pixel_size, int weight) {
  auto button_font = font();
  button_font.setPixelSize(pixel_size);
  button_font.setWeight(weight);
  setFont(button_font);
}

void FlatButton::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
      if(!event->isAutoRepeat()) {
        Q_EMIT clicked(isChecked());
      }
      break;
    default:
      QPushButton::keyPressEvent(event);
  }
}

QSize FlatButton::sizeHint() const {
  return scale(180, 26);
}

void FlatButton::update_style() {
  auto default_style_sheet = QString(R"(
    #flat_button {
      background-color: %1;
      border: %2px solid %3;
      color: %4;
      margin: 0px;
      padding: 0px;
    })")
    .arg(m_default_style.m_background_color.name(QColor::HexArgb))
    .arg(scale_width(1))
    .arg(m_default_style.m_border_color.name(QColor::HexArgb))
    .arg(m_default_style.m_text_color.name(QColor::HexArgb));
  auto hover_style_sheet = QString(R"(
    #flat_button:hover {
      background-color: %1;
      border-color: %2;
      color: %3;
    }
    #flat_button:focus:hover {
      background-color: %1;
      border-color: %2;
      color: %3;
    })")
    .arg(m_hover_style.m_background_color.name(QColor::HexArgb))
    .arg(m_hover_style.m_border_color.name(QColor::HexArgb))
    .arg(m_hover_style.m_text_color.name(QColor::HexArgb));
  auto focus_style_sheet = QString(R"(
    #flat_button:focus {
      background-color: %1;
      border-color: %2;
      color: %3;
      outline: none;
    })")
    .arg(m_focus_style.m_background_color.name(QColor::HexArgb))
    .arg(m_focus_style.m_border_color.name(QColor::HexArgb))
    .arg(m_focus_style.m_text_color.name(QColor::HexArgb));
  auto disabled_style_sheet = QString(R"(
    #flat_button:disabled {
      background-color: %1;
      border-color: %2;
      color: %3;
    })")
    .arg(m_disabled_style.m_background_color.name(QColor::HexArgb))
    .arg(m_disabled_style.m_border_color.name(QColor::HexArgb))
    .arg(m_disabled_style.m_text_color.name(QColor::HexArgb));
  auto style_sheet = QString();
  style_sheet.reserve(default_style_sheet.length() +
    hover_style_sheet.length() + focus_style_sheet.length() +
    disabled_style_sheet.length());
  setStyleSheet(style_sheet.append(default_style_sheet).
    append(hover_style_sheet).append(focus_style_sheet).
    append(disabled_style_sheet));
}
