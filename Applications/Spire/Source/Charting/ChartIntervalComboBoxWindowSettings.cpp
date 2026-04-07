#include "Spire/Charting/ChartIntervalComboBoxWindowSettings.hpp"
#include "Spire/Charting/ChartIntervalComboBox.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

ChartIntervalComboBoxWindowSettings::ChartIntervalComboBoxWindowSettings(
    const ChartIntervalComboBox& widget)
    : m_type(widget.GetType()),
      m_value(widget.GetValue()),
      m_geometry(widget.saveGeometry()) {}

QWidget* ChartIntervalComboBoxWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  auto comboBox = new ChartIntervalComboBox();
  Apply(Ref(userProfile), out(*comboBox));
  return comboBox;
}

void ChartIntervalComboBoxWindowSettings::Apply(
    Ref<UserProfile> userProfile, Out<QWidget> widget) const {
  auto& comboBox = dynamic_cast<ChartIntervalComboBox&>(*widget);
  comboBox.restoreGeometry(m_geometry);
  comboBox.SetInterval(m_type, m_value);
}
