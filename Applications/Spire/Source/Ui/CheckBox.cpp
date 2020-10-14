#include "Spire/Ui/CheckBox.hpp"
#include <QFocusEvent>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

CheckBox::CheckBox(const QString& text, QWidget* parent)
  : QCheckBox(text, parent),
    m_last_focus_reason(Qt::MouseFocusReason) {}

void CheckBox::set_stylesheet(const QString& text_style,
    const QString& indicator_style, const QString& checked_style,
    const QString& hover_style, const QString& focused_style) {
  m_text_style = QString("QCheckBox { %1 }").arg(text_style);
  m_indicator_style = QString("QCheckBox::indicator { %1 }").
    arg(indicator_style);
  m_checked_style = QString("QCheckBox::indicator:checked { %1 }")
    .arg(checked_style);
  m_hover_style = QString("QCheckBox::indicator:hover { %1 }")
    .arg(hover_style);
  m_focused_style = QString("QCheckBox::indicator { %1 }")
    .arg(focused_style);
  if(m_last_focus_reason == Qt::MouseFocusReason) {
    set_hover_stylesheet();
  } else {
    set_focused_stylesheet();
  }
}

void CheckBox::focusInEvent(QFocusEvent* event)  {
  if(event->reason() == Qt::ActiveWindowFocusReason) {
    if(m_last_focus_reason == Qt::MouseFocusReason) {
      set_hover_stylesheet();
    } else {
      set_focused_stylesheet();
    }
    return;
  } else if(event->reason() != Qt::MouseFocusReason) {
    set_focused_stylesheet();
  } else {
    set_hover_stylesheet();
  }
  m_last_focus_reason = event->reason();
}

void CheckBox::focusOutEvent(QFocusEvent* event) {
  set_hover_stylesheet();
}

void CheckBox::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    set_hover_stylesheet();
    m_last_focus_reason = Qt::MouseFocusReason;
  }
  QCheckBox::mouseReleaseEvent(event);
}

void CheckBox::set_hover_stylesheet() {
  setStyleSheet(m_text_style + m_indicator_style + m_checked_style +
    m_hover_style);
}

void CheckBox::set_focused_stylesheet() {
  setStyleSheet(m_text_style + m_indicator_style + m_checked_style +
    m_focused_style);
}

CheckBox* Spire::make_check_box(const QString& label, QWidget* parent) {
  auto check_box = new CheckBox(label, parent);
  auto check_box_text_style = QString(R"(
    color: black;
    font-family: Roboto;
    font-size: %1px;
    outline: none;
    spacing: %2px;)")
    .arg(scale_height(12)).arg(scale_width(4));
  auto check_box_indicator_style = QString(R"(
    background-color: white;
    border: %1px solid #C8C8C8 %2px solid #C8C8C8;
    height: %3px;
    width: %4px;)").arg(scale_height(1))
    .arg(scale_width(1)).arg(scale_height(15)).arg(scale_width(15));
  auto check_box_checked_style = QString(R"(
    image: url(:/Icons/check-with-box.svg);)");
  auto check_box_hover_style = QString(R"(
    border: %1px solid #4B23A0 %2px solid #4B23A0;)")
    .arg(scale_height(1)).arg(scale_width(1));
  auto check_box_focused_style = QString(R"(border-color: #4B23A0;)");
  check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  return check_box;
}
