#include "Spire/Ui/MoneyBox.hpp"
#include <boost/lexical_cast.hpp>
#include <QHBoxLayout>
#include "Spire/Ui/LocalScalarValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  DecimalBox::Decimal to_decimal(Money money) {
    return DecimalBox::Decimal(lexical_cast<std::string>(money));
  }

  optional<DecimalBox::Decimal> to_decimal(optional<Money> money) {
    if(money) {
      return to_decimal(*money);
    }
    return none;
  }
}

struct MoneyBox::MoneyToDecimalModel : DecimalBox::DecimalModel {
  mutable CurrentSignal m_current_signal;
  std::shared_ptr<OptionalMoneyModel> m_model;
  optional<DecimalBox::Decimal> m_current;
  scoped_connection m_current_connection;

  MoneyToDecimalModel(std::shared_ptr<OptionalMoneyModel> model)
    : m_model(std::move(model)),
      m_current(to_decimal(m_model->get_current())),
      m_current_connection(m_model->connect_current_signal(
        [=] (const auto& current) {
          on_current(current);
        })) {}

  optional<DecimalBox::Decimal> get_minimum() const {
    return to_decimal(m_model->get_minimum());
  }

  optional<DecimalBox::Decimal> get_maximum() const {
    return to_decimal(m_model->get_maximum());
  }

  DecimalBox::Decimal get_increment() const {
    return DecimalBox::Decimal("0.000001");
  }

  QValidator::State get_state() const override {
    return m_model->get_state();
  }

  const optional<DecimalBox::Decimal>& get_current() const {
    return m_current;
  }

  QValidator::State set_current(
      const optional<DecimalBox::Decimal>& value) override {
    auto state = [&] {
      if(value) {
        return m_model->set_current(Money::FromValue(value->str()));
      }
      return m_model->set_current(none);
    }();
    if(state == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    m_current = value;
    m_current_signal(value);
    return state;
  }

  connection connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const override {
    return m_current_signal.connect(slot);
  }

  void on_current(const optional<Money>& current) {
    auto decimal = to_decimal(current);
    m_current = decimal;
    m_current_signal(decimal);
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
  auto adapted_modifiers = QHash<Qt::KeyboardModifier, DecimalBox::Decimal>();
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

void MoneyBox::on_submit(const optional<DecimalBox::Decimal>& submission) {
  m_submission = m_model->get_current();
  m_submit_signal(m_submission);
}

void MoneyBox::on_reject(const optional<DecimalBox::Decimal>& value) {
  if(value) {
    m_reject_signal(Money::FromValue(value->str()));
  } else {
    m_reject_signal(none);
  }
}
