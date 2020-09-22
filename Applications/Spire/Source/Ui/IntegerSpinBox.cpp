#include "Spire/Ui/IntegerSpinBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/RealSpinBoxModel.hpp"

using namespace boost::signals2;
using namespace Spire;

IntegerSpinBox::IntegerSpinBox(
    std::shared_ptr<IntegerSpinBoxModel> model, QWidget* parent)
    : QAbstractSpinBox(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_spin_box = new RealSpinBox(
    std::make_unique<RealSpinBoxModelImpl<std::int64_t>>(std::move(model)),
    this);
  m_spin_box->set_decimal_places(0);
  m_spin_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusProxy(m_spin_box);
  m_spin_box->connect_change_signal([=] (auto value) {
    m_change_signal(value.extract_signed_long_long());
  });
  connect(m_spin_box, &RealSpinBox::editingFinished, this,
    &IntegerSpinBox::editingFinished);
}

connection IntegerSpinBox::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

std::int64_t IntegerSpinBox::get_value() const {
  return m_spin_box->get_value().extract_signed_long_long();
}

void IntegerSpinBox::set_value(std::int64_t value) {
  m_spin_box->set_value(value);
}
