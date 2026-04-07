#include "Spire/PortfolioViewer/PortfolioViewerWindowSettings.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "Ui_PortfolioViewerWindow.h"

using namespace Beam;
using namespace Spire;
using namespace std;

PortfolioViewerWindowSettings::PortfolioViewerWindowSettings() {}

PortfolioViewerWindowSettings::PortfolioViewerWindowSettings(
    const PortfolioViewerWindow& window)
    : m_name("Portfolio Viewer"),
      m_properties(window.m_properties),
      m_geometry(window.saveGeometry()),
      m_portfolioTableHeaderState(
        window.m_ui->m_portfolioTableView->horizontalHeader()->saveState()),
      m_splitterState(window.m_ui->m_viewSplitter->saveState()),
      m_selectionTableHeaderState(
        window.m_ui->m_selectionTreeView->header()->saveState()) {}

PortfolioViewerWindowSettings::~PortfolioViewerWindowSettings() {}

string PortfolioViewerWindowSettings::GetName() const {
  return m_name;
}

QWidget* PortfolioViewerWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  PortfolioViewerWindow* window = new PortfolioViewerWindow(Ref(userProfile),
    m_properties);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(userProfile), out(*window));
  return window;
}

void PortfolioViewerWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  PortfolioViewerWindow& window = dynamic_cast<PortfolioViewerWindow&>(*widget);
  window.restoreGeometry(m_geometry);
  window.m_ui->m_portfolioTableView->horizontalHeader()->restoreState(
    m_portfolioTableHeaderState);
  window.m_ui->m_viewSplitter->restoreState(m_splitterState);
  window.m_ui->m_selectionTreeView->header()->restoreState(
    m_selectionTableHeaderState);
}
