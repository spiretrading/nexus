#include "Spire/Charting/ChartWindowSettings.hpp"
#include "Spire/Charting/ChartIntervalComboBox.hpp"
#include "Spire/Charting/ChartWindow.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_ChartWindow.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

ChartWindowSettings::ChartWindowSettings() {}

ChartWindowSettings::ChartWindowSettings(const ChartWindow& window,
    Ref<UserProfile> userProfile)
    : m_interactionMode(window.GetInteractionMode()),
      m_isAutoScaleEnabled(window.IsAutoScaleEnabled()),
      m_isLockGridEnabled(window.IsLockGridEnabled()),
      m_security(window.GetDisplayedSecurity()),
      m_securityViewStack(window.m_securityViewStack),
      m_identifier(window.GetIdentifier()),
      m_linkIdentifier(window.m_linkIdentifier),
      m_geometry(window.saveGeometry()),
      m_chartPlotViewWindowSettings(window.m_ui->m_chart->GetWindowSettings()),
      m_chartIntervalComboBoxWindowSettings(
        window.m_intervalComboBox->GetWindowSettings()) {
  if(m_security == Security()) {
    m_name = "Chart";
  } else {
    m_name = "Chart - " +
      ToString(m_security, userProfile->GetMarketDatabase());
  }
}

ChartWindowSettings::~ChartWindowSettings() {}

string ChartWindowSettings::GetName() const {
  return m_name;
}

QWidget* ChartWindowSettings::Reopen(Ref<UserProfile> userProfile) const {
  ChartWindow* window = new ChartWindow(Ref(userProfile), m_identifier);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(userProfile), Store(*window));
  return window;
}

void ChartWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  ChartWindow& window = dynamic_cast<ChartWindow&>(*widget);
  window.restoreGeometry(m_geometry);
  if(window.m_ui->m_chart != nullptr) {
    m_chartPlotViewWindowSettings->Apply(Ref(userProfile),
      Store(*window.m_ui->m_chart));
  }
  window.SetInteractionMode(m_interactionMode);
  window.SetAutoScale(m_isAutoScaleEnabled);
  window.SetLockGrid(m_isLockGridEnabled);
  if(m_security != Security()) {
    window.DisplaySecurity(m_security);
  }
  window.m_linkIdentifier = m_linkIdentifier;
  window.m_securityViewStack = m_securityViewStack;
  if(window.m_intervalComboBox != nullptr) {
    m_chartIntervalComboBoxWindowSettings->Apply(Ref(userProfile),
      Store(*window.m_intervalComboBox));
  }
}
