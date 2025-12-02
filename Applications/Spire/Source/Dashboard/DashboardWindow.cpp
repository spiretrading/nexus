#include "Spire/Dashboard/DashboardWindow.hpp"
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <QMessageBox>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include "Spire/CanvasView/OrderTaskView.hpp"
#include "Spire/Dashboard/DashboardModel.hpp"
#include "Spire/Dashboard/DashboardModelSchema.hpp"
#include "Spire/Dashboard/DashboardRenderer.hpp"
#include "Spire/Dashboard/DashboardRow.hpp"
#include "Spire/Dashboard/DashboardSelectionModel.hpp"
#include "Spire/Dashboard/DashboardWindowSettings.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/Dashboard/TextDashboardCellRenderer.hpp"
#include "Spire/Dashboard/ValueDashboardCell.hpp"
#include "Spire/LegacyUI/LineInputDialog.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_DashboardWindow.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

string DashboardWindow::GetDefaultName() {
  return "";
}

DashboardWindow::DashboardWindow(const string& name,
    const DashboardModelSchema& schema, Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_ui{std::make_unique<Ui_DashboardWindow>()},
      m_userProfile{userProfile.get()} {
  m_ui->setupUi(this);
  auto displayTaskSlot = [=] (CondensedCanvasWidget& widget) {
    m_ui->verticalLayout->insertWidget(2, &widget);
    widget.Focus();
  };
  auto removeTaskSlot = [=] (CondensedCanvasWidget& widget) {
    m_ui->verticalLayout->removeWidget(&widget);
    m_ui->m_dashboard->setFocus();
  };
  m_orderTaskView.emplace(displayTaskSlot, removeTaskSlot, Ref(*this),
    Ref(*m_userProfile));
  setFocusProxy(m_ui->m_dashboard);
  auto& savedDashboards = m_userProfile->GetSavedDashboards();
  for(auto& entry : savedDashboards.GetDashboards()) {
    m_ui->m_savesComboBox->addItem(QString::fromStdString(entry.m_name));
  }
  Apply(schema, name);
  connect(m_ui->m_savesComboBox, &QComboBox::textActivated, this,
    &DashboardWindow::OnDashboardActivated);
  connect(m_ui->m_saveButton, &QPushButton::pressed, this,
    &DashboardWindow::OnSaveButtonPressed);
  connect(m_ui->m_deleteButton, &QPushButton::pressed, this,
    &DashboardWindow::OnDeleteButtonPressed);
  m_rowAddedConnection = m_model->ConnectRowAddedSignal(
    std::bind(&DashboardWindow::OnRowAdded, this, std::placeholders::_1));
  m_rowRemovedConnection = m_model->ConnectRowRemovedSignal(
    std::bind(&DashboardWindow::OnRowRemoved, this, std::placeholders::_1));
  m_dashboardAddedConnection =
    m_userProfile->GetSavedDashboards().ConnectDashboardAddedSignal(
    std::bind(&DashboardWindow::OnDashboardAdded, this, std::placeholders::_1));
  m_dashboardRemovedConnection =
    m_userProfile->GetSavedDashboards().ConnectDashboardRemovedSignal(
    std::bind(&DashboardWindow::OnDashboardRemoved, this,
    std::placeholders::_1));
}

DashboardWindow::~DashboardWindow() {}

std::unique_ptr<WindowSettings> DashboardWindow::GetWindowSettings() const {
  return std::make_unique<DashboardWindowSettings>(*this);
}

void DashboardWindow::showEvent(QShowEvent* event) {
  auto showData = JsonObject();
  showData["id"] = reinterpret_cast<std::intptr_t>(this);
}

void DashboardWindow::closeEvent(QCloseEvent* event) {
  Save();
  auto window = GetWindowSettings();
  m_userProfile->GetRecentlyClosedWindows()->push(std::move(window));
  auto closeData = JsonObject();
  closeData["id"] = reinterpret_cast<std::intptr_t>(this);
  QWidget::closeEvent(event);
}

void DashboardWindow::keyPressEvent(QKeyEvent* event) {
  if(!m_ui->m_dashboard->hasFocus()) {
    m_ui->m_dashboard->setFocus();
    QApplication::sendEvent(m_ui->m_dashboard, event);
    return;
  }
  auto security = GetActiveSecurity();
  if(!security.is_initialized()) {
    return QWidget::keyPressEvent(event);
  }
  auto bboQuote = [&] {
    auto bboQuoteIterator = m_bboQuotes.find(*security);
    if(bboQuoteIterator == m_bboQuotes.end()) {
      return BboQuote();
    }
    return bboQuoteIterator->second.m_bboQuote->peek();
  }();
  if(m_orderTaskView->HandleKeyPressEvent(*event, *security,
      bboQuote.m_ask.m_price, bboQuote.m_bid.m_price)) {
    return;
  }
  QWidget::keyPressEvent(event);
}

boost::optional<Security> DashboardWindow::GetActiveSecurity() const {
  auto activeRow = m_ui->m_dashboard->GetSelectionModel().GetActiveRow();
  if(!activeRow.is_initialized()) {
    return none;
  }
  auto modelRow = m_ui->m_dashboard->GetRenderer().GetRow(*activeRow);
  if(!modelRow.is_initialized()) {
    return none;
  }
  auto& values = modelRow->GetIndex().GetValues();
  if(values.empty()) {
    return none;
  }
  auto security = boost::get<Security>(&values.back());
  if(security == nullptr) {
    return none;
  }
  return *security;
}

void DashboardWindow::Save() {
  if(m_name.empty()) {
    return;
  }
  SavedDashboards::Entry entry{m_name, {*m_model,
    m_ui->m_dashboard->GetRowBuilder()},
    m_ui->m_dashboard->GetWindowSettings()};
  auto& savedDashboards = m_userProfile->GetSavedDashboards();
  savedDashboards.Save(entry);
}

void DashboardWindow::SetName(const string& name) {
  m_name = name;
  if(m_name.empty()) {
    setWindowTitle(tr("Dashboard - Spire"));
    m_ui->m_deleteButton->setEnabled(false);
    m_ui->m_savesComboBox->setCurrentIndex(-1);
  } else {
    setWindowTitle(tr("Dashboard - ") + QString::fromStdString(m_name));
    m_ui->m_deleteButton->setEnabled(true);
    auto& savedDashboards = m_userProfile->GetSavedDashboards();
    for(auto i = 0;
        i < static_cast<int>(savedDashboards.GetDashboards().size()); ++i) {
      if(savedDashboards.GetDashboards()[i].m_name == m_name) {
        m_ui->m_savesComboBox->setCurrentIndex(i);
        break;
      }
    }
  }
}

void DashboardWindow::Apply(const DashboardModelSchema& schema,
    const string& name) {
  m_model = schema.Make(Ref(*m_userProfile));
  m_ui->m_dashboard->Initialize(Ref(*m_model), schema.GetRowBuilder(),
    Ref(*m_userProfile));
  SetName(name);
}

void DashboardWindow::OnRowAdded(const DashboardRow& row) {
  auto& values = row.GetIndex().GetValues();
  if(values.empty()) {
    return;
  }
  auto security = boost::get<Security>(&values.back());
  if(security == nullptr) {
    return;
  }
  auto& bboQuoteEntry = get_or_insert(m_bboQuotes, *security);
  if(bboQuoteEntry.m_counter == 0) {
    bboQuoteEntry.m_bboQuote = std::make_shared<StateQueue<BboQuote>>();
    bboQuoteEntry.m_bboQuote->push(BboQuote());
    auto query = make_current_query(*security);
    m_userProfile->GetClients().get_market_data_client().query(
      query, bboQuoteEntry.m_bboQuote);
  }
  ++bboQuoteEntry.m_counter;
}

void DashboardWindow::OnRowRemoved(const DashboardRow& row) {
  auto& values = row.GetIndex().GetValues();
  if(values.empty()) {
    return;
  }
  auto security = boost::get<Security>(&values.back());
  if(security == nullptr) {
    return;
  }
  auto bboQuoteIterator = m_bboQuotes.find(*security);
  if(bboQuoteIterator == m_bboQuotes.end()) {
    return;
  }
  auto& bboQuoteEntry = bboQuoteIterator->second;
  --bboQuoteEntry.m_counter;
  if(bboQuoteEntry.m_counter == 0) {
    m_bboQuotes.erase(bboQuoteIterator);
  }
}

void DashboardWindow::OnDashboardAdded(const SavedDashboards::Entry& entry) {
  m_ui->m_savesComboBox->addItem(QString::fromStdString(entry.m_name));
  if(entry.m_name == m_name) {
    m_ui->m_savesComboBox->setCurrentIndex(m_ui->m_savesComboBox->count() - 1);
  }
}

void DashboardWindow::OnDashboardRemoved(const SavedDashboards::Entry& entry) {
  for(auto i = 0; i < m_ui->m_savesComboBox->count(); ++i) {
    if(m_ui->m_savesComboBox->itemText(i).toStdString() == entry.m_name) {
      m_ui->m_savesComboBox->removeItem(i);
      break;
    }
  }
}

void DashboardWindow::OnDashboardActivated(const QString& text) {
  auto& savedDashboards = m_userProfile->GetSavedDashboards();
  auto name = text.toStdString();
  if(name == m_name) {
    return;
  }
  for(auto& savedDashboard : savedDashboards.GetDashboards()) {
    if(savedDashboard.m_name == name) {
      Save();
      Apply(savedDashboard.m_schema, savedDashboard.m_name);
      savedDashboard.m_settings->Apply(Ref(*m_userProfile),
        out(*m_ui->m_dashboard));
      break;
    }
  }
}

void DashboardWindow::OnSaveButtonPressed() {
  auto& savedDashboards = m_userProfile->GetSavedDashboards();
  while(true) {
    LineInputDialog inputDialog{"Save Dashboard", "Name", "", this};
    if(inputDialog.exec() == QDialog::Rejected) {
      return;
    }
    auto name = boost::trim_copy(inputDialog.GetInput());
    auto exists = false;
    for(auto& savedDashboard : savedDashboards.GetDashboards()) {
      if(savedDashboard.m_name == name) {
        exists = true;
        QMessageBox::critical(nullptr, QObject::tr("Error"),
          QObject::tr("Dashboard already exists."));
        break;
      }
    }
    if(!exists) {
      SetName(name);
      Save();
      m_ui->m_deleteButton->setEnabled(true);
      break;
    }
  }
}

void DashboardWindow::OnDeleteButtonPressed() {
  if(m_name.empty()) {
    return;
  }
  auto& savedDashboards = m_userProfile->GetSavedDashboards();
  savedDashboards.Delete(m_name);
  SetName("");
}
