#include "Spire/Ui/MoneyBox.hpp"
#include <boost/lexical_cast.hpp>
#include <QHBoxLayout>
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/DecimalBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

struct MoneyBox::MoneyToDecimalModel : ToDecimalModel<Money> {
  using ToDecimalModel<Money>::ToDecimalModel;

  Decimal get_increment() const override {
    return Decimal("0.000001");
  }
};

MoneyBox::MoneyBox(QHash<Qt::KeyboardModifier, Money> modifiers,
  QWidget* parent)
  : MoneyBox(std::make_shared<LocalOptionalMoneyModel>(),
      std::move(modifiers), parent) {}

MoneyBox::MoneyBox(std::shared_ptr<OptionalMoneyModel> model,
    QHash<Qt::KeyboardModifier, Money> modifiers, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_adaptor_model(std::make_shared<MoneyToDecimalModel>(m_model)),
      m_submission(m_model->get_current()) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto adapted_modifiers = QHash<Qt::KeyboardModifier, Decimal>();
  for(auto modifier = modifiers.begin();
      modifier != modifiers.end(); ++modifier) {
    adapted_modifiers.insert(modifier.key(), to_decimal(modifier.value()));
  }
  m_decimal_box = new DecimalBox(m_adaptor_model, adapted_modifiers, this);
  proxy_style(*this, *m_decimal_box);
  setFocusProxy(m_decimal_box);
  layout->addWidget(m_decimal_box);
  auto style = get_style(*m_decimal_box);
  style.get(Any()).set(TrailingZeros(2));
  set_style(*m_decimal_box, std::move(style));
  m_submit_connection = m_decimal_box->connect_submit_signal(
    [=] (const auto& submission) { on_submit(submission); });
  m_reject_connection = m_decimal_box->connect_reject_signal(
    [=] (const auto& value) { on_reject(value); });
}

const std::shared_ptr<OptionalMoneyModel>& MoneyBox::get_model() const {
  return m_model;
}

void MoneyBox::set_placeholder(const QString& value) {
  m_decimal_box->set_placeholder(value);
}

bool MoneyBox::is_read_only() const {
  return m_decimal_box->is_read_only();
}

void MoneyBox::set_read_only(bool is_read_only) {
  m_decimal_box->set_read_only(is_read_only);
}

connection MoneyBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection MoneyBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

void MoneyBox::on_submit(const optional<Decimal>& submission) {
  m_submission = m_model->get_current();
  m_submit_signal(m_submission);
}

void MoneyBox::on_reject(const optional<Decimal>& value) {
  m_reject_signal(from_decimal<Money>(value));
}
