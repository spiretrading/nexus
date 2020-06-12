#include "Spire/Ui/ColorSelectorHexInputWidget.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QRegExpValidator>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

ColorSelectorHexInputWidget::ColorSelectorHexInputWidget(
    const QColor& current_color, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto hex_label = new QLabel(tr("HEX"), this);
  hex_label->setFixedSize(scale(31, 26));
  layout->addWidget(hex_label);
  m_line_edit = new QLineEdit("#", this);
  m_line_edit->setValidator(new QRegExpValidator(QRegExp("^#[0-9a-fA-F]{1,6}$"),
    this));
  connect(m_line_edit, &QLineEdit::textChanged, this,
    &ColorSelectorHexInputWidget::on_text_changed);
  m_line_edit->setFixedSize(scale(91, 26));
  layout->addWidget(m_line_edit);
}

void ColorSelectorHexInputWidget::on_text_changed(const QString& text) {
  if(!text.startsWith("#")) {
    m_line_edit->setText("#");
  }
}
