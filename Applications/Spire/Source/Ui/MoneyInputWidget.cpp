#include "Spire/Ui/MoneyInputWidget.hpp"
#include <QHBoxLayout>
#include <QRegularExpressionValidator>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

MoneyInputWidget::MoneyInputWidget(QWidget* parent)
    : QLineEdit(parent),
      m_item_delegate(this) {
  apply_line_edit_style(this);
  setValidator(new QRegularExpressionValidator(
    QRegularExpression("((\\d+\\.?\\d{0,2})|(\\.\\d{0,2}))"), this));
  connect(this, &QLineEdit::editingFinished, this,
    &MoneyInputWidget::on_line_edit_committed);
  connect(this, &QLineEdit::textEdited, this,
    &MoneyInputWidget::on_line_edit_modified);
}

void MoneyInputWidget::set_value(Money value) {
  setText(m_item_delegate.displayText(
    QVariant::fromValue(Round(value, 2)), QLocale()));
}

connection MoneyInputWidget::connect_committed_signal(
    const InputSignal::slot_type& slot) const {
  return m_committed_signal.connect(slot);
}

connection MoneyInputWidget::connect_modified_signal(
    const InputSignal::slot_type& slot) const {
  return m_modified_signal.connect(slot);
}

void MoneyInputWidget::on_line_edit_committed() {
  if(!text().isEmpty()) {
    auto value = Money::FromValue(text().toStdString());
    if(value) {
      m_committed_signal(*value);
    }
  }
}

void MoneyInputWidget::on_line_edit_modified(const QString& text) {
  if(!text.isEmpty()) {
    auto value = Money::FromValue(text.toStdString());
    if(value) {
      m_modified_signal(*value);
    }
  }
}
