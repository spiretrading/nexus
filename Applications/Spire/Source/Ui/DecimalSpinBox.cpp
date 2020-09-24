#include "Spire/Ui/DecimalSpinBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/RealSpinBoxModel.hpp"
#include "Spire/Spire/SpinBoxModel.hpp"

using namespace boost::signals2;
using namespace Spire;

DecimalSpinBox::DecimalSpinBox(
    std::shared_ptr<DecimalSpinBoxModel> model, QWidget* parent)
    : QAbstractSpinBox(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_spin_box = new RealSpinBox(
    std::make_unique<RealSpinBoxModelImpl<double>>(std::move(model)), this);
  setFocusProxy(m_spin_box);
  m_spin_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_spin_box);
  m_spin_box->connect_change_signal([=] (auto value) {
    m_change_signal(value.extract_double());
  });
  connect(m_spin_box, &RealSpinBox::editingFinished, this,
    &DecimalSpinBox::editingFinished);
}

connection DecimalSpinBox::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

double DecimalSpinBox::get_value() const {
  return m_spin_box->get_value().extract_double();
}

void DecimalSpinBox::set_value(double value) {
  m_spin_box->set_value(static_cast<long double>(value));
}
