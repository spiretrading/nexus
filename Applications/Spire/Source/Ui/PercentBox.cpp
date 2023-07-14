#include "Spire/Ui/PercentBox.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto to_percentage(const optional<Decimal>& value) {
    if(value) {
      return optional<Decimal>(*value * 100);
    }
    return value;
  }

  auto to_quantity(const optional<Decimal>& value) {
    if(value) {
      return optional<Decimal>(*value / 100);
    }
    return value;
  }

  struct OptionalPercentModel : OptionalDecimalModel {
    using UpdateSignal = typename OptionalDecimalModel::UpdateSignal;
    mutable typename UpdateSignal m_update_signal;
    std::shared_ptr<OptionalDecimalModel> m_model;
    optional<Decimal> m_value;
    scoped_connection m_connection;

    OptionalPercentModel(std::shared_ptr<OptionalDecimalModel> model)
      : m_model(std::move(model)),
        m_value(m_model->get()),
        m_connection(m_model->connect_update_signal(
          std::bind_front(&OptionalPercentModel::on_update, this))) {}

    QValidator::State get_state() const {
      return m_model->get_state();
    }

    optional<Decimal> get_minimum() const {
      return to_percentage(m_model->get_minimum());
    }

    optional<Decimal> get_maximum() const {
      return to_percentage(m_model->get_maximum());
    }

    Decimal get_increment() const {
      return m_model->get_increment();
    }

    const optional<Decimal>& get() const override {
      return m_value;
    }

    QValidator::State test(const optional<Decimal>& value) const override {
      return m_model->test(to_quantity(value));
    }

    QValidator::State set(const optional<Decimal>& value) override {
      m_value = value;
      return m_model->set(to_quantity(value));
    }

    connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
      return m_update_signal.connect(slot);
    }

    void on_update(const optional<Decimal>& value) {
      m_value = to_percentage(value);
      m_update_signal(m_value);
    }
  };
}

PercentBox::PercentBox(QWidget* parent)
  : PercentBox(QHash<Qt::KeyboardModifier, Decimal>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
      {Qt::ShiftModifier, 20}}), parent) {}

PercentBox::PercentBox(
    QHash<Qt::KeyboardModifier, Decimal> modifiers, QWidget* parent)
  : PercentBox(std::make_shared<LocalOptionalDecimalModel>(),
    std::move(modifiers), parent) {}

PercentBox::PercentBox(std::shared_ptr<OptionalDecimalModel> model,
    QHash<Qt::KeyboardModifier, Decimal> modifiers, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  m_decimal_box = new DecimalBox(
    std::make_shared<OptionalPercentModel>(m_model), std::move(modifiers));
  enclose(*this, *m_decimal_box);
  setFocusProxy(m_decimal_box);
  proxy_style(*this, *m_decimal_box);
  update_style(*m_decimal_box, [] (auto& style) {});
  m_submit_connection = m_decimal_box->connect_submit_signal(
    std::bind_front(&PercentBox::on_submit, this));
  m_reject_connection = m_decimal_box->connect_reject_signal(
    std::bind_front(&PercentBox::on_reject, this));
}

const std::shared_ptr<OptionalDecimalModel>& PercentBox::get_current() const {
  return m_model;
}

void PercentBox::set_placeholder(const QString& value) {
  m_decimal_box->set_placeholder(value);
}

bool PercentBox::is_read_only() const {
  return m_decimal_box->is_read_only();
}

void PercentBox::set_read_only(bool is_read_only) {
  m_decimal_box->set_read_only(is_read_only);
}

connection PercentBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection PercentBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

void PercentBox::on_submit(const optional<Decimal>& submission) {
  m_submit_signal(to_quantity(submission));
}

void PercentBox::on_reject(const optional<Decimal>& value) {
  m_reject_signal(to_quantity(value));
}
