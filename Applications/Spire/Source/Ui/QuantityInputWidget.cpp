#include "Spire/Ui/QuantityInputWidget.hpp"
#include <QHBoxLayout>
#include <QRegularExpressionValidator>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

QuantityInputWidget::QuantityInputWidget(QWidget* parent)
    : TextInputWidget(parent),
      m_item_delegate(this) {
  setValidator(new QRegularExpressionValidator(
    QRegularExpression("^[0-9]*$"), this));
  connect(this, &QLineEdit::editingFinished, this,
    &QuantityInputWidget::on_line_edit_committed);
  connect(this, &QLineEdit::textEdited, this,
    &QuantityInputWidget::on_line_edit_modified);
  m_locale.setNumberOptions(QLocale::OmitGroupSeparator);
}

void QuantityInputWidget::set_value(Quantity value) {
  setText(m_item_delegate.displayText(
    QVariant::fromValue(Truncate(value, 0)), m_locale));
}

connection QuantityInputWidget::connect_committed_signal(
    const InputSignal::slot_type& slot) const {
  return m_committed_signal.connect(slot);
}

connection QuantityInputWidget::connect_modified_signal(
    const InputSignal::slot_type& slot) const {
  return m_modified_signal.connect(slot);
}

void QuantityInputWidget::on_line_edit_committed() {
  if(!text().isEmpty()) {
    m_committed_signal(*Quantity::FromValue(text().toStdString()));
  }
}

void QuantityInputWidget::on_line_edit_modified(const QString& text) {
  if(!text.isEmpty()) {
    m_modified_signal(*Quantity::FromValue(text.toStdString()));
  }
}
