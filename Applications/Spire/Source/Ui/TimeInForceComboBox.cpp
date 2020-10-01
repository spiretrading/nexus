#include "Spire/Ui/TimeInForceComboBox.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <QHBoxLayout>
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeInForceComboBox::TimeInForceComboBox(QWidget* parent)
    : QWidget(parent) {
  auto items = [] {
    auto times = std::vector<QVariant>();
    times.reserve(TimeInForce::Type::COUNT);
    for(auto time : Beam::MakeRange<TimeInForce::Type>()) {
      times.push_back(QVariant::fromValue<TimeInForce>(time));
    }
    return times;
  }();
  auto menu = new StaticDropDownMenu(items, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(menu);
  m_value_connection = menu->connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<TimeInForce>());
    });
}

connection TimeInForceComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
