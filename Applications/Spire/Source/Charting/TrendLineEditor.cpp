#include "Spire/Charting/TrendLineEditor.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include "Spire/Charting/TrendLineStyleDropDownMenu.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"

using namespace boost::signals2;
using namespace Spire;

TrendLineEditor::TrendLineEditor(const RecentColors& recent_colors,
    QWidget* parent)
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
  draw_tool_label->setStyleSheet(QString(R"(
    color: #000000;
    font-family: Roboto;
    font-size: %1px;
  )").arg(scale_height(10)));
  layout->addWidget(draw_tool_label);
  layout->addStretch(8);
  auto color_button = new ColorSelectorButton(QColor("#FFCA19"),
    recent_colors, this);
  color_button->connect_color_signal(
    [=] (auto color) { on_color_change(color); });
  color_button->connect_recent_colors_signal(
    [=] (const auto& recent_colors) {
      on_recent_colors_changed(recent_colors);
    });
  color_button->setFixedSize(scale(70, 18));
  layout->addWidget(color_button);
  layout->addStretch(8);
  auto style_dropdown = new TrendLineStyleDropDownMenu(this);
  style_dropdown->setFixedSize(scale(70, 18));
  style_dropdown->connect_style_signal(
    [=] (auto style) { on_style_change(style); });
  layout->addWidget(style_dropdown);
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

void TrendLineEditor::set_style(TrendLineStyle style) {
  m_line_style = style;
}

connection TrendLineEditor::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

connection TrendLineEditor::connect_recent_colors_signal(
    const RecentColorsSignal::slot_type& slot) const {
  return m_recent_colors_signal.connect(slot);
}

connection TrendLineEditor::connect_style_signal(
    const StyleSignal::slot_type& slot) const {
  return m_style_signal.connect(slot);
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

void TrendLineEditor::on_color_change(const QColor& color) {
  m_color = color;
  m_color_signal();
}

void TrendLineEditor::on_recent_colors_changed(
    const RecentColors& recent_colors) {
  m_recent_colors_signal(recent_colors);
}

void TrendLineEditor::on_style_change(TrendLineStyle style) {
  m_line_style = style;
  m_style_signal();
}

void TrendLineEditor::move_to_parent() {
  auto parent_widget = static_cast<QWidget*>(parent());
  auto parent_pos = parent_widget->geometry().topRight();
  move(parent_pos.x() - width() - scale_width(8),
    parent_pos.y() + scale_height(8));
  raise();
}
