#include "spire/charting/trend_line_editor.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/dropdown_color_picker.hpp"

using namespace Spire;

TrendLineEditor::TrendLineEditor(QWidget* parent)
  : QWidget(parent),
    m_color(QColor("#FFCA19")),
    m_line_style(TrendLineStyle::SOLID) {
  parent->installEventFilter(this);
  setFixedSize(scale(216, 34));
  setStyleSheet("background-color: #F5F5F5");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(scale_width(8), scale_height(8),
    scale_width(8), scale_width(8));
  auto draw_tool_label = new QLabel(tr("Draw Tool"), this);
  layout->addWidget(draw_tool_label);
  layout->addStretch(8);
  auto color_picker = new DropdownColorPicker(this);
  color_picker->setFixedSize(scale(70, 18));
  layout->addWidget(color_picker);
  layout->addStretch(8);
}

const QColor& TrendLineEditor::get_color() const {
  return m_color;
}

void TrendLineEditor::set_color(const QColor& color) {
  m_color = color;
}

TrendLineStyle TrendLineEditor::get_style() const {
  return m_line_style;
}

void TrendLineEditor::set_style(TrendLineStyle style) const {
  m_line_style = style;
}

bool TrendLineEditor::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Move || event->type() == QEvent::Resize) {
    move_to_parent();
  }
  return QWidget::eventFilter(watched, event);
}

void TrendLineEditor::showEvent(QShowEvent* event) {
  move_to_parent();
}

void TrendLineEditor::move_to_parent() {
  auto parent_widget = static_cast<QWidget*>(parent());
  auto parent_pos = parent_widget->geometry().topRight();
  move(parent_pos.x() - width() - scale_width(8),
    parent_pos.y() + scale_height(8));
  raise();
}
