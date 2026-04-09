#include "Spire/TimeAndSales/TimeAndSalesWindowSettings.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_TimeAndSalesWindow.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

TimeAndSalesWindowSettings::TimeAndSalesWindowSettings() {}

TimeAndSalesWindowSettings::TimeAndSalesWindowSettings(
    const TimeAndSalesWindow& window, Ref<UserProfile> userProfile)
    : m_properties(window.GetProperties()),
      m_ticker(window.m_ticker),
      m_tickerViewStack(window.m_tickerViewStack),
      m_identifier(window.GetIdentifier()),
      m_linkIdentifier(window.m_linkIdentifier),
      m_geometry(window.saveGeometry()),
      m_splitterState(window.m_ui->m_splitter->saveState()),
      m_viewHeaderState(
        window.m_ui->m_timeAndSalesView->horizontalHeader()->saveState()),
      m_snapshotHeaderState(
        window.m_ui->m_snapshotView->horizontalHeader()->saveState()) {
  if(m_ticker == Ticker()) {
    m_name = "Time And Sales";
  } else {
    m_name = "Time And Sales - " + displayText(m_ticker).toStdString();
  }
}

TimeAndSalesWindowSettings::~TimeAndSalesWindowSettings() {}

string TimeAndSalesWindowSettings::GetName() const {
  return m_name;
}

QWidget* TimeAndSalesWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  TimeAndSalesWindow* window = new TimeAndSalesWindow(Ref(userProfile),
    m_properties, m_identifier);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(userProfile), out(*window));
  return window;
}

void TimeAndSalesWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  TimeAndSalesWindow& window = dynamic_cast<TimeAndSalesWindow&>(*widget);
  window.restoreGeometry(m_geometry);
  window.m_ui->m_splitter->restoreState(m_splitterState);
  window.m_ui->m_timeAndSalesView->horizontalHeader()->restoreState(
    m_viewHeaderState);
  window.m_ui->m_snapshotView->horizontalHeader()->restoreState(
    m_snapshotHeaderState);
  window.m_tickerViewStack = m_tickerViewStack;
  if(m_ticker != Ticker()) {
    window.DisplayTicker(m_ticker);
  }
  window.m_linkIdentifier = m_linkIdentifier;
}
