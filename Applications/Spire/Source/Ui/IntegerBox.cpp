#include "Spire/Ui/IntegerBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/DecimalBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

IntegerBox::IntegerBox(QHash<Qt::KeyboardModifier, int> modifiers,
  QWidget* parent)
  : IntegerBox(std::make_shared<LocalOptionalIntegerModel>(),
      std::move(modifiers), parent) {}

IntegerBox::IntegerBox(std::shared_ptr<OptionalIntegerModel> model,
    QHash<Qt::KeyboardModifier, int> modifiers, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_adaptor_model(std::make_shared<ToDecimalModel<int>>(m_model)),
      m_submission(m_model->get_current()) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto adapted_modifiers = QHash<Qt::KeyboardModifier, Decimal>();
  for(auto modifier = modifiers.begin();
      modifier != modifiers.end(); ++modifier) {
    adapted_modifiers.insert(modifier.key(), modifier.value());
  }
  m_decimal_box = new DecimalBox(m_adaptor_model, adapted_modifiers, this);
  proxy_style(*this, *m_decimal_box);
  setFocusProxy(m_decimal_box);
  layout->addWidget(m_decimal_box);
  m_submit_connection = m_decimal_box->connect_submit_signal(
    [=] (const auto& submission) { on_submit(submission); });
  m_reject_connection = m_decimal_box->connect_reject_signal(
    [=] (const auto& value) { on_reject(value); });
}

const std::shared_ptr<OptionalIntegerModel>& IntegerBox::get_model() const {
  return m_model;
}

void IntegerBox::set_placeholder(const QString& value) {
  m_decimal_box->set_placeholder(value);
}

bool IntegerBox::is_read_only() const {
  return m_decimal_box->is_read_only();
}

void IntegerBox::set_read_only(bool is_read_only) {
  m_decimal_box->set_read_only(is_read_only);
}

connection IntegerBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection IntegerBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

void IntegerBox::on_submit(const optional<Decimal>& submission) {
  m_submission = m_model->get_current();
  m_submit_signal(m_submission);
}

void IntegerBox::on_reject(const optional<Decimal>& value) {
  if(value) {
    m_reject_signal(value->convert_to<int>());
  } else {
    m_reject_signal(none);
  }
}
