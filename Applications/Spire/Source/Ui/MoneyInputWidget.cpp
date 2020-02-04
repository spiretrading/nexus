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
    QRegularExpression("((\d+\.?\d*)|(\.\d+))"), this));
  connect(this, &QLineEdit::editingFinished, this,
    &MoneyInputWidget::on_line_edit_committed);
  connect(this, &QLineEdit::textEdited, this,
    &MoneyInputWidget::on_line_edit_modified);
}

void MoneyInputWidget::set_value(Money value) {
  setText(m_item_delegate.displayText(
    QVariant::fromValue(Truncate(value, 0)), QLocale()));
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
    m_committed_signal(*Money::FromValue(text().toStdString()));
  }
}

void MoneyInputWidget::on_line_edit_modified(const QString& text) {
  if(!text.isEmpty()) {
    m_modified_signal(*Money::FromValue(text.toStdString()));
  }
}
