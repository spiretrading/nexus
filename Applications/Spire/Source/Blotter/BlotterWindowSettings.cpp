#include "Spire/Blotter/BlotterWindowSettings.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_BlotterWindow.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

BlotterWindowSettings::BlotterWindowSettings() {}

BlotterWindowSettings::BlotterWindowSettings(const BlotterWindow& window)
    : m_blotterName(window.GetModel().GetName()),
      m_geometry(window.saveGeometry()),
      m_splitterState(window.m_ui->m_splitter->saveState()),
      m_openPositionsWidgetState(
        window.m_ui->m_openPositionsTab->GetUIState()),
      m_orderLogWidgetState(window.m_ui->m_orderLogTab->GetUIState()),
      m_activityLogWidgetState(window.m_ui->m_activityLogTab->GetUIState()) {}

BlotterWindowSettings::~BlotterWindowSettings() {}

const string& BlotterWindowSettings::GetBlotterName() const {
  return m_blotterName;
}

string BlotterWindowSettings::GetName() const {
  return "Blotter - " + m_blotterName;
}

QWidget* BlotterWindowSettings::Reopen(Ref<UserProfile> userProfile) const {
  BlotterModel* model = nullptr;
  BlotterSettings& settings = userProfile->GetBlotterSettings();
  for(auto i = settings.GetAllBlotters().begin();
      i != settings.GetAllBlotters().end(); ++i) {
    if((*i)->GetName() == m_blotterName) {
      model = i->get();
      break;
    }
  }
  if(model == nullptr) {
    return nullptr;
  }
  BlotterWindow* window = &BlotterWindow::GetBlotterWindow(Ref(userProfile),
    Ref(*model));
  Apply(Ref(userProfile), Store(*window));
  return window;
}

void BlotterWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  BlotterWindow& window = dynamic_cast<BlotterWindow&>(*widget);
  restore_geometry(window, m_geometry);
  window.m_ui->m_splitter->restoreState(m_splitterState);
  window.m_ui->m_openPositionsTab->SetUIState(m_openPositionsWidgetState);
  window.m_ui->m_orderLogTab->SetUIState(m_orderLogWidgetState);
  window.m_ui->m_activityLogTab->SetUIState(m_activityLogWidgetState);
}
