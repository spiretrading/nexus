#include "Spire/UI/ImportExportDialog.hpp"
#include <QStandardPaths>
#include "Spire/UI/EnvironmentSettings.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/Toolbar.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_ImportExportDialog.h"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

ImportExportDialog::ImportExportDialog(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_ImportExportDialog>()),
      m_userProfile(userProfile.Get()) {
  m_ui->setupUi(this);
  if(!m_userProfile->IsManager()) {
    m_ui->m_portfolioViewerSettingsButton->setChecked(false);
    m_ui->m_portfolioViewerSettingsButton->hide();
  }
  path settingsPath = QStandardPaths::writableLocation(
    QStandardPaths::DocumentsLocation).toStdString();
  settingsPath /= userProfile->GetUsername() + "_settings.sps";
  m_ui->m_pathInput->SetPath(settingsPath);
  m_ui->m_pathInput->SetCaption("Select the settings file.");
  m_ui->m_pathInput->SetFilter("Settings (*.sps)");
  setFixedSize(size());
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &ImportExportDialog::close);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &ImportExportDialog::CommitSettings);
  connect(m_ui->m_importButton, &QRadioButton::toggled, this,
    &ImportExportDialog::SetOpenMode);
  connect(m_ui->m_importAndApplyButton, &QRadioButton::toggled, this,
    &ImportExportDialog::SetOpenMode);
  connect(m_ui->m_exportButton, &QRadioButton::toggled, this,
    &ImportExportDialog::SetSaveMode);
}

ImportExportDialog::~ImportExportDialog() {}

void ImportExportDialog::CommitSettings() {
  if(m_ui->m_exportButton->isChecked()) {
    ExportSettings();
  } else {
    ImportSettings();
  }
}

void ImportExportDialog::ExportSettings() {
  EnvironmentSettings environmentSettings;
  if(m_ui->m_bookViewSettingsButton->isChecked()) {
    environmentSettings.m_bookViewProperties =
      m_userProfile->GetDefaultBookViewProperties();
  }
  if(m_ui->m_dashboardsButton->isChecked()) {
    environmentSettings.m_dashboards = m_userProfile->GetSavedDashboards();
  }
  if(m_ui->m_orderImbalanceIndicatorButton->isChecked()) {
    environmentSettings.m_orderImbalanceIndicatorProperties =
      m_userProfile->GetDefaultOrderImbalanceIndicatorProperties();
  }
  if(m_ui->m_interactionsButton->isChecked()) {
    environmentSettings.m_interactionsProperties =
      m_userProfile->GetInteractionProperties();
  }
  if(m_ui->m_keyBindingsButton->isChecked()) {
    environmentSettings.m_keyBindings = m_userProfile->GetKeyBindings();
  }
  if(m_ui->m_portfolioViewerSettingsButton->isChecked()) {
    environmentSettings.m_portfolioViewerProperties =
      m_userProfile->GetDefaultPortfolioViewerProperties();
  }
  if(m_ui->m_timeAndSalesSettingsButton->isChecked()) {
    environmentSettings.m_timeAndSalesProperties =
      m_userProfile->GetDefaultTimeAndSalesProperties();
  }
  if(m_ui->m_windowLayoutButton->isChecked()) {
    vector<std::shared_ptr<WindowSettings>> windowLayouts;
    for(auto widget : QApplication::topLevelWidgets()) {
      auto window = dynamic_cast<PersistentWindow*>(widget);
      if(window != nullptr && dynamic_cast<Toolbar*>(widget) == nullptr) {
        windowLayouts.push_back(window->GetWindowSettings());
      }
    }
    environmentSettings.m_windowLayouts = windowLayouts;
  }
  if(Export(environmentSettings, m_ui->m_pathInput->GetPath())) {
    accept();
  }
}

void ImportExportDialog::ImportSettings() {
  EnvironmentSettings::TypeSet settings;
  if(m_ui->m_bookViewSettingsButton->isChecked()) {
    settings.Set(EnvironmentSettings::Type::BOOK_VIEW);
  }
  if(m_ui->m_dashboardsButton->isChecked()) {
    settings.Set(EnvironmentSettings::Type::DASHBOARDS);
  }
  if(m_ui->m_orderImbalanceIndicatorButton->isChecked()) {
    settings.Set(EnvironmentSettings::Type::ORDER_IMBALANCE_INDICATOR);
  }
  if(m_ui->m_interactionsButton->isChecked()) {
    settings.Set(EnvironmentSettings::Type::INTERACTIONS);
  }
  if(m_ui->m_keyBindingsButton->isChecked()) {
    settings.Set(EnvironmentSettings::Type::KEY_BINDINGS);
  }
  if(m_ui->m_portfolioViewerSettingsButton->isChecked()) {
    settings.Set(EnvironmentSettings::Type::PORTFOLIO_VIEWER);
  }
  if(m_ui->m_timeAndSalesSettingsButton->isChecked()) {
    settings.Set(EnvironmentSettings::Type::TIME_AND_SALES);
  }
  if(m_ui->m_windowLayoutButton->isChecked()) {
    settings.Set(EnvironmentSettings::Type::WINDOW_LAYOUTS);
  }
  if(Import(m_ui->m_pathInput->GetPath(), settings,
      m_ui->m_importAndApplyButton->isChecked(), Store(*m_userProfile))) {
    accept();
  }
}

void ImportExportDialog::SetOpenMode(bool toggled) {
  if(toggled) {
    m_ui->m_pathInput->SetAcceptMode(QFileDialog::AcceptOpen);
  }
}

void ImportExportDialog::SetSaveMode(bool toggled) {
  if(toggled) {
    m_ui->m_pathInput->SetAcceptMode(QFileDialog::AcceptSave);
  }
}
