#include "Spire/Ui/QuantityInputWidget.hpp"
#include <QHBoxLayout>
#include <QRegularExpressionValidator>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

QuantityInputWidget::QuantityInputWidget(QWidget* parent)
    : QWidget(parent),
      m_item_delegate(this) {
  setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  m_line_edit = new InputWidget(this);
  m_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_line_edit->setValidator(new QRegularExpressionValidator(
    QRegularExpression("^[0-9]*$"), this));
  layout->addWidget(m_line_edit);
  connect(m_line_edit, &QLineEdit::editingFinished, this,
    &QuantityInputWidget::on_line_edit_committed);
  connect(m_line_edit, &QLineEdit::textEdited, this,
    &QuantityInputWidget::on_line_edit_modified);
}

void QuantityInputWidget::set_value(Quantity value) {
  m_line_edit->setText(m_item_delegate.displayText(
    QVariant::fromValue(Truncate(value, 0)), QLocale()));
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
  if(!m_line_edit->text().isEmpty()) {
    m_committed_signal(*Quantity::FromValue(m_line_edit->text().toStdString()));
  }
}

void QuantityInputWidget::on_line_edit_modified(const QString& text) {
  if(!m_line_edit->text().isEmpty()) {
    m_modified_signal(*Quantity::FromValue(text.toStdString()));
  }
}
