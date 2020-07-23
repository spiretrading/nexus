#include "Spire/Ui/FlatButton.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointF>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using Style = FlatButton::Style;

FlatButton::FlatButton(QWidget* parent)
  : FlatButton("", parent) {}

FlatButton::FlatButton(const QString& label, QWidget* parent)
    : QWidget(parent),
      m_clickable(true),
      m_last_focus_reason(Qt::MouseFocusReason) {
  setObjectName("flat_button");
  m_label = new QLabel(this);
  m_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  auto layout = new QHBoxLayout(this);
  set_label(label);
  layout->addWidget(m_label);
  setFocusPolicy(Qt::StrongFocus);
  m_default_style.m_background_color = Qt::white;
  m_default_style.m_border_color = Qt::transparent;
  m_default_style.m_text_color = Qt::black;
  m_hover_style.m_background_color = Qt::white;
  m_hover_style.m_border_color = Qt::transparent;
  m_hover_style.m_text_color = Qt::black;
  m_focus_style.m_background_color = Qt::white;
  m_focus_style.m_border_color = Qt::transparent;
  m_focus_style.m_text_color = Qt::black;
  m_disabled_style.m_background_color = Qt::transparent;
  m_disabled_style.m_border_color = Qt::white;
  m_disabled_style.m_text_color = Qt::white;
  set_style(m_default_style);
  set_hover_style(m_hover_style);
  set_focus_style(m_focus_style);
  set_disabled_style(m_disabled_style);
}

void FlatButton::set_label(const QString& text) {
  if(text.isEmpty()) {
    layout()->setContentsMargins(scale_width(2), scale_height(2),
      scale_width(2), scale_height(2));
  } else {
    layout()->setContentsMargins({});
  }
  m_label->setText(text);
}

const Style& FlatButton::get_style() const {
  return m_default_style;
}

void FlatButton::set_style(const Style& default_style) {
  m_default_style = default_style;
  on_style_updated();
}

const Style& FlatButton::get_hover_style() const {
  return m_hover_style;
}

void FlatButton::set_hover_style(const Style& hover_style) {
  m_hover_style = hover_style;
  on_style_updated();
}

const Style& FlatButton::get_focus_style() const {
  return m_focus_style;
}

void FlatButton::set_focus_style(const Style& focus_style) {
  m_focus_style = focus_style;
  on_style_updated();
}

const Style& FlatButton::get_disabled_style() const {
  return m_disabled_style;
}

void FlatButton::set_disabled_style(const Style& disabled_style) {
  m_disabled_style = disabled_style;
  on_style_updated();
}

connection FlatButton::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void FlatButton::changeEvent(QEvent* event) {
  if(event->type() == QEvent::FontChange) {
    m_label->setFont(font());
  } else if(event->type() == QEvent::EnabledChange) {
    if(!isEnabled()) {
      disable_button();
    } else {
      enable_button();
    }
  }
}

void FlatButton::focusInEvent(QFocusEvent* event) {
  if(event->reason() == Qt::ActiveWindowFocusReason) {
    if(m_last_focus_reason == Qt::MouseFocusReason) {
      set_hover_stylesheet();
    } else {
      set_focus_stylesheet();
    }
    return;
  } else if(event->reason() != Qt::MouseFocusReason) {
    set_focus_stylesheet();
  } else {
    set_hover_stylesheet();
  }
  m_last_focus_reason = event->reason();
}

void FlatButton::focusOutEvent(QFocusEvent* event) {
  set_hover_stylesheet();
}

void FlatButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    if(m_clickable) {
      m_clicked_signal();
    }
  }
}

void FlatButton::mousePressEvent(QMouseEvent* event) {
  event->accept();
}

void FlatButton::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && m_clickable) {
    set_hover_stylesheet();
    m_last_focus_reason = Qt::MouseFocusReason;
    if(rect().contains(event->localPos().toPoint())) {
      m_clicked_signal();
    }
  }
}

void FlatButton::disable_button() {
  m_clickable = false;
  setFocusPolicy(Qt::NoFocus);
  set_disabled_stylesheet();
}

void FlatButton::enable_button() {
  m_clickable = true;
  setFocusPolicy(Qt::StrongFocus);
  set_hover_stylesheet();
}

QString FlatButton::get_stylesheet_properties(const Style& s) {
  auto label_border_style = [&] {
    if(!m_label->text().isEmpty()) {
      return QString("border: %1px solid %2 %3px solid %2;")
        .arg(scale_width(1))
        .arg(s.m_border_color.name(QColor::HexArgb))
        .arg(scale_height(1));
    }
    return QString();
  }();
  return QString(R"(
    background-color: %1;
    color: %2;
    qproperty-alignment: AlignCenter;
    %3)")
    .arg(s.m_background_color.name(QColor::HexArgb))
    .arg(s.m_text_color.name(QColor::HexArgb))
    .arg(label_border_style);
}

void FlatButton::set_disabled_stylesheet() {
  setStyleSheet(QString(R"(
    #flat_button {
      border: %1px solid %2 %3px solid %2;
    })")
    .arg(scale_height(1))
    .arg(m_disabled_style.m_border_color.name(QColor::HexArgb))
    .arg(scale_width(1)));
  m_label->setStyleSheet(get_stylesheet_properties(m_disabled_style));
}

void FlatButton::set_focus_stylesheet() {
  setStyleSheet(QString(R"(
    #flat_button {
      border: %1px solid %2 %3px solid %2;
    })")
    .arg(scale_height(1))
    .arg(m_focus_style.m_border_color.name(QColor::HexArgb))
    .arg(scale_width(1)));
  m_label->setStyleSheet(
    QString(R"(QLabel { %1 })").arg(
      get_stylesheet_properties(m_focus_style)) +
    QString(R"(QLabel:hover { %1 })").arg(
      get_stylesheet_properties(m_hover_style)));
}

void FlatButton::set_hover_stylesheet() {
  setStyleSheet(QString(R"(
    #flat_button {
      background-color: #FFFFFF;
      border: %1px solid %2 %3px solid %2;
    }

    #flat_button:hover {
      border: %1px solid %4 %3px solid %4;
    })").arg(scale_height(1))
    .arg(m_default_style.m_border_color.name(QColor::HexArgb))
    .arg(scale_width(1))
    .arg(m_hover_style.m_border_color.name(QColor::HexArgb)));
  m_label->setStyleSheet(
    QString(R"(QLabel { %1 })").arg(
      get_stylesheet_properties(m_default_style)) +
    QString(R"(QLabel:hover { %1 })").arg(
      get_stylesheet_properties(m_hover_style)));
}

void FlatButton::on_style_updated() {
  if(!hasFocus() && isEnabled()) {
    set_hover_stylesheet();
  } else if(hasFocus()) {
    set_focus_stylesheet();
  } else {
    set_disabled_stylesheet();
  }
}
