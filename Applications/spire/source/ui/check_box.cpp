#include "spire/ui/check_box.hpp"
#include <QFocusEvent>

using namespace spire;

check_box::check_box(const QString& text, QWidget* parent)
  : QCheckBox(text, parent) {}

void check_box::set_stylesheet(const QString& default_style,
    const QString& hover_style, const QString& focused_style) {
  m_default_style = default_style;
  m_hover_style = hover_style;
  m_focused_style = focused_style;
  set_hover_stylesheet();
}

void check_box::focusInEvent(QFocusEvent* event)  {
  if(event->reason() == Qt::TabFocusReason ||
      event->reason() == Qt::BacktabFocusReason) {
    set_focused_stylesheet();
  }
}

void check_box::focusOutEvent(QFocusEvent* event) {
  set_hover_stylesheet();
}

void check_box::set_hover_stylesheet() {
  setStyleSheet(m_default_style + m_hover_style);
}

void check_box::set_focused_stylesheet() {
  setStyleSheet(m_default_style + m_focused_style);
}
