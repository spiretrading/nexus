#include "Spire/Ui/MoneySpinBox.hpp"
#include <QHBoxLayout>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_money(RealSpinBox::Real value) {
    if(auto money = Money::FromValue(value.str(
        std::numeric_limits<Money>::digits10, std::ios_base::dec))) {
      return *money;
    }
    throw std::runtime_error("MoneySpinBox: failed to convert Real to Money.");
  }
}

MoneySpinBox::MoneySpinBox(Money value, QWidget* parent)
    : QAbstractSpinBox(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_spin_box = new RealSpinBox(to_real(value), this);
  m_spin_box->set_step(static_cast<long double>(0.01));
  m_spin_box->set_minimum_decimal_places(2);
  m_spin_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusProxy(m_spin_box);
  layout->addWidget(m_spin_box);
  m_spin_box->connect_change_signal([=] (auto value) {
    m_change_signal(to_money(value));
  });
  connect(m_spin_box, &RealSpinBox::editingFinished, this,
    &MoneySpinBox::editingFinished);
  m_locale.setNumberOptions(m_locale.numberOptions().setFlag(
    QLocale::OmitGroupSeparator, true));
}

connection MoneySpinBox::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

void MoneySpinBox::set_minimum(Money minimum) {
  m_spin_box->set_minimum(to_real(minimum));
}

void MoneySpinBox::set_maximum(Money maximum) {
  m_spin_box->set_maximum(to_real(maximum));
}

Money MoneySpinBox::get_value() const {
  return to_money(m_spin_box->get_value());
}

void MoneySpinBox::set_value(Money value) {
  m_spin_box->set_value(to_real(value));
}

RealSpinBox::Real MoneySpinBox::to_real(Nexus::Money value) {
  return m_item_delegate.displayText(
    QVariant::fromValue(value), m_locale).toStdString().c_str();
}
