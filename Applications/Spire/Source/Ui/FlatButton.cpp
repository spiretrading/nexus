#include "Spire/Ui/FlatButton.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

FlatButton::FlatButton(QWidget* parent)
  : FlatButton("", parent) {}

FlatButton::FlatButton(const QString& label, QWidget* parent)
    : QWidget(parent) {
  setObjectName("flat_button");
  m_label = new QLabel(this);
  m_label->setObjectName("flat_button_label");
  m_label->setAlignment(Qt::AlignCenter);
  m_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  auto layout = new QHBoxLayout(this);
  set_label(label);
  layout->addWidget(m_label);
  layout->setContentsMargins({});
  setFocusPolicy(Qt::StrongFocus);
  auto button_font = QFont("Roboto");
  button_font.setPixelSize(scale_height(12));
  button_font.setWeight(QFont::Normal);
  button_font.setLetterSpacing(QFont::AbsoluteSpacing, 0);
  m_label->setFont(button_font);
  set_style({QColor("#EBEBEB"), QColor("#EBEBEB"), QColor("#000000")},
    {QColor("#4B23A0"), QColor("#4B23A0"), QColor("#FFFFFF")},
    {QColor("#EBEBEB"), QColor("#4B23A0"), QColor("#000000")},
    {QColor("#EBEBEB"), QColor("#EBEBEB"), QColor("#A0A0A0")});
}

void FlatButton::set_label(const QString& label) {
  m_label->setText(label);
}

void FlatButton::set_style(const Style& default_style, const Style& hover_style,
    const Style& focus_style, const Style& disabled_style) {
  m_default_style = default_style;
  m_hover_style = hover_style;
  m_focus_style = focus_style;
  m_disabled_style = disabled_style;
  setStyleSheet(QString(R"(
    #flat_button {
      background-color: %1;
      border: %2px solid %3;
      margin: 0px;
    }
    #flat_button:hover {
      background-color: %4;
      border-color: %5;
    }
    #flat_button:focus:hover {
      background-color: %4;
      border-color: %5;
    }
    #flat_button:focus {
      background-color: %6;
      border-color: %7;
    }
    #flat_button:disabled {
      background-color: %8;
      border-color: %9;
    })")
    .arg(m_default_style.m_background_color.name(QColor::HexArgb))
    .arg(scale_width(1))
    .arg(m_default_style.m_border_color.name(QColor::HexArgb))
    .arg(m_hover_style.m_background_color.name(QColor::HexArgb))
    .arg(m_hover_style.m_border_color.name(QColor::HexArgb))
    .arg(m_focus_style.m_background_color.name(QColor::HexArgb))
    .arg(m_focus_style.m_border_color.name(QColor::HexArgb))
    .arg(m_disabled_style.m_background_color.name(QColor::HexArgb))
    .arg(m_disabled_style.m_border_color.name(QColor::HexArgb)));
  set_label_stylesheet(false);
}

const FlatButton::Style& FlatButton::get_default_style() const {
  return m_default_style;
}

void FlatButton::set_default_style(const Style& default_style) {
  set_style(default_style, m_hover_style, m_focus_style, m_disabled_style);
}

const FlatButton::Style& FlatButton::get_hover_style() const {
  return m_hover_style;
}

void FlatButton::set_hover_style(const Style& hover_style) {
  set_style(m_default_style, hover_style, m_focus_style, m_disabled_style);
}

const FlatButton::Style& FlatButton::get_focus_style() const {
  return m_focus_style;
}

void FlatButton::set_focus_style(const Style& focus_style) {
  set_style(m_default_style, m_hover_style, focus_style, m_disabled_style);
}

const FlatButton::Style& FlatButton::get_disabled_style() const {
  return m_disabled_style;
}

void FlatButton::set_disabled_style(const Style& disabled_style) {
  set_style(m_default_style, m_hover_style, m_focus_style, disabled_style);
}

void FlatButton::set_font_size_weight(int font_pixel_size, int font_weight) {
  auto button_font = m_label->font();
  button_font.setPixelSize(font_pixel_size);
  button_font.setWeight(font_weight);
  m_label->setFont(button_font);
}

connection FlatButton::connect_pressed_signal(
    const PressedSignal::slot_type& slot) const {
  return m_pressed_signal.connect(slot);
}

void FlatButton::changeEvent(QEvent* event) {
  if(event->type() == QEvent::FontChange) {
    m_label->setFont(font());
  }
}

void FlatButton::focusInEvent(QFocusEvent* event) {
  set_label_stylesheet(true);
}

void FlatButton::focusOutEvent(QFocusEvent* event) {
  set_label_stylesheet(false);
}

void FlatButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    m_pressed_signal();
  }
  QWidget::keyPressEvent(event);
}

void FlatButton::mousePressEvent(QMouseEvent* event) {
  event->accept();
}

void FlatButton::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(rect().contains(event->localPos().toPoint())) {
      m_pressed_signal();
    }
  }
}

QSize FlatButton::sizeHint() const {
  return scale(180, 26);
}

void FlatButton::set_label_stylesheet(bool is_focus) {
  auto text_color = m_default_style.m_text_color;
  if(is_focus) {
    text_color = m_focus_style.m_text_color;
  }
  m_label->setStyleSheet(QString(R"(
    #flat_button_label {
      background-color: #00000000;
      color: %1;
    }
    #flat_button_label:hover {
      color: %2;
    }
    #flat_button_label:disabled {
      color: %3;
    })")
    .arg(text_color.name(QColor::HexArgb))
    .arg(m_hover_style.m_text_color.name(QColor::HexArgb))
    .arg(m_disabled_style.m_text_color.name(QColor::HexArgb)));
}
