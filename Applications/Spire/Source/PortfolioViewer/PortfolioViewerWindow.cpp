#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStatusBar>
#include "Spire/AccountViewer/TraderProfileWindow.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/PortfolioViewer/PortfolioSelectionModel.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindowSettings.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerModel.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/FunctionalAction.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/LegacyUI/ValueLabel.hpp"
#include "Spire/Utilities/ExportModel.hpp"
#include "ui_PortfolioViewerWindow.h"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  void FlattenPosition(const PortfolioViewerModel::Entry& entry,
      UserProfile& userProfile) {
    auto& blotter = userProfile.GetBlotterSettings().GetConsolidatedBlotter(
      entry.m_account);
    auto orderFields = OrderFields::MakeMarketOrder(
      blotter.GetExecutingAccount(),
      entry.m_inventory.m_position.m_key.m_index,
      entry.m_inventory.m_position.m_key.m_currency,
      GetOpposite(GetSide(entry.m_inventory.m_position)),
      userProfile.GetDestinationDatabase().GetPreferredDestination(
      entry.m_inventory.m_position.m_key.m_index.GetMarket()).m_id,
      Abs(entry.m_inventory.m_position.m_quantity));
    auto orderNode = MakeOrderTaskNodeFromOrderFields(orderFields,
      userProfile);
    auto& taskEntry = blotter.GetTasksModel().Add(*orderNode);
    taskEntry.m_task->Execute();
  }
}

PortfolioViewerWindow::PortfolioViewerWindow(Ref<UserProfile> userProfile,
    const PortfolioViewerProperties& properties, QWidget* parent,
    Qt::WindowFlags flags)
    : QFrame(parent, flags),
      m_ui(std::make_unique<Ui_PortfolioViewerWindow>()),
      m_userProfile(userProfile.Get()) {
  m_ui->setupUi(this);
  m_statusBar = new QStatusBar(this);
  m_statusBar->setStyleSheet("QStatusBar::item { border: 0px solid black };");
  auto spacer = new QLabel("");
  QSizePolicy spacerSizePolicy(QSizePolicy::MinimumExpanding,
    QSizePolicy::Minimum);
  spacerSizePolicy.setHorizontalStretch(1);
  spacer->setSizePolicy(spacerSizePolicy);
  m_statusBar->addWidget(spacer);
  m_totalProfitAndLossLabel = new ValueLabel("Total P/L", Ref(*m_userProfile));
  m_totalProfitAndLossLabel->AdjustSize("9999999.99");
  m_statusBar->addWidget(m_totalProfitAndLossLabel);
  m_unrealizedProfitAndLossLabel = new ValueLabel("Unrealized P/L",
    Ref(*m_userProfile));
  m_unrealizedProfitAndLossLabel->AdjustSize("9999999.99");
  m_statusBar->addWidget(m_unrealizedProfitAndLossLabel);
  m_realizedProfitAndLossLabel = new ValueLabel("Realized P/L",
    Ref(*m_userProfile));
  m_realizedProfitAndLossLabel->AdjustSize("9999999.99");
  m_statusBar->addWidget(m_realizedProfitAndLossLabel);
  m_feesLabel = new ValueLabel("Fees", Ref(*m_userProfile));
  m_realizedProfitAndLossLabel->AdjustSize("9999999.99");
  m_statusBar->addWidget(m_feesLabel);
  m_volumeLabel = new ValueLabel("Volume", Ref(*m_userProfile));
  m_volumeLabel->AdjustSize("9999999999");
  m_statusBar->addWidget(m_volumeLabel);
  m_tradesLabel = new ValueLabel("Trades", Ref(*m_userProfile));
  m_tradesLabel->AdjustSize("9999999999");
  m_statusBar->addWidget(m_tradesLabel);
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_statusBar->sizePolicy().hasHeightForWidth());
  m_statusBar->setSizePolicy(sizePolicy);
  m_ui->verticalLayout->addWidget(m_statusBar);
  m_ui->m_selectionTreeView->setItemDelegate(new CustomVariantItemDelegate(
    Ref(userProfile)));
  m_ui->m_portfolioTableView->setItemDelegate(new CustomVariantItemDelegate(
    Ref(userProfile)));
  m_ui->m_portfolioTableView->horizontalHeader()->setSectionsMovable(true);
  m_ui->m_portfolioTableView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ui->m_portfolioTableView, &QTableView::customContextMenuRequested,
    this, &PortfolioViewerWindow::OnContextMenu);
  SetProperties(properties);
}

PortfolioViewerWindow::~PortfolioViewerWindow() {}

void PortfolioViewerWindow::SetProperties(
    const PortfolioViewerProperties& properties) {
  m_properties = properties;
  m_selectionModel = std::make_unique<PortfolioSelectionModel>(
    Ref(*m_userProfile), m_properties);
  m_ui->m_selectionTreeView->setModel(m_selectionModel.get());
  m_viewerModel = std::make_unique<PortfolioViewerModel>(Ref(*m_userProfile),
    Ref(*m_selectionModel));
  m_proxyViewerModel = std::make_unique<CustomVariantSortFilterProxyModel>(
    Ref(*m_userProfile));
  m_proxyViewerModel->setSourceModel(m_viewerModel.get());
  m_ui->m_portfolioTableView->setModel(m_proxyViewerModel.get());
  QFontMetrics metrics(m_ui->m_portfolioTableView->font());
  m_ui->m_portfolioTableView->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  m_totalsUpdatedConnection = m_viewerModel->ConnectTotalsUpdatedSignal(
    std::bind(&PortfolioViewerWindow::OnTotalsUpdated, this,
    std::placeholders::_1));
}

unique_ptr<WindowSettings> PortfolioViewerWindow::GetWindowSettings() const {
  m_selectionModel->UpdateProperties(Store(m_properties));
  auto settings = std::make_unique<PortfolioViewerWindowSettings>(*this);
  return std::move(settings);
}

void PortfolioViewerWindow::showEvent(QShowEvent* event) {
  QFrame::showEvent(event);
  m_ui->m_portfolioTableView->horizontalHeader()->setStretchLastSection(true);
}

void PortfolioViewerWindow::closeEvent(QCloseEvent* event) {
  m_selectionModel->UpdateProperties(Store(m_properties));
  auto settings = std::make_unique<PortfolioViewerWindowSettings>(*this);
  m_userProfile->SetInitialPortfolioViewerWindowSettings(*settings);
  m_userProfile->SetDefaultPortfolioViewerProperties(m_properties);
  m_userProfile->AddRecentlyClosedWindow(std::move(settings));
  QFrame::closeEvent(event);
}

void PortfolioViewerWindow::OnTotalsUpdated(
    const PortfolioViewerModel::TotalEntry& totals) {
  m_totalProfitAndLossLabel->SetValue(QVariant::fromValue(
    totals.m_realizedProfitAndLoss + totals.m_unrealizedProfitAndLoss));
  m_realizedProfitAndLossLabel->SetValue(QVariant::fromValue(
    totals.m_realizedProfitAndLoss));
  m_unrealizedProfitAndLossLabel->SetValue(QVariant::fromValue(
    totals.m_unrealizedProfitAndLoss));
  m_feesLabel->SetValue(QVariant::fromValue(totals.m_fees));
  m_volumeLabel->SetValue(QVariant::fromValue(totals.m_volume));
  m_tradesLabel->SetValue(totals.m_trades);
}

void PortfolioViewerWindow::OnContextMenu(const QPoint& position) {
  auto selectedRows =
    m_ui->m_portfolioTableView->selectionModel()->selectedRows();
  unordered_set<DirectoryEntry> selectedAccounts;
  vector<PortfolioViewerModel::Entry> positions;
  for(auto& index : selectedRows) {
    auto sourceIndex = m_proxyViewerModel->mapToSource(index);
    auto& entry = m_viewerModel->GetEntry(sourceIndex.row());
    selectedAccounts.insert(entry.m_account);
    if(entry.m_inventory.m_position.m_quantity != 0) {
      positions.push_back(entry);
    }
  }
  auto contextMenu = std::make_unique<QMenu>();
  if(!selectedAccounts.empty()) {
    auto blotterAction = new FunctionalAction(contextMenu.get());
    blotterAction->SetFunction(
      [&] {
        for(auto& account : selectedAccounts) {
          auto& blotter =
            m_userProfile->GetBlotterSettings().GetConsolidatedBlotter(account);
          auto& window = BlotterWindow::GetBlotterWindow(Ref(*m_userProfile),
            Ref(blotter));
          window.show();
        }
      });
    QString blotterText;
    if(selectedAccounts.size() == 1) {
      blotterText = tr("Open Blotter");
    } else {
      blotterText = tr("Open Blotters");
    }
    blotterAction->setText(blotterText);
    contextMenu->addAction(blotterAction);
    auto profileAction = new FunctionalAction(contextMenu.get());
    profileAction->SetFunction(
      [&] {
        for(auto& account : selectedAccounts) {
          auto profileWindow = new TraderProfileWindow(Ref(*m_userProfile));
          profileWindow->setAttribute(Qt::WA_DeleteOnClose);
          profileWindow->Load(account);
          profileWindow->show();
        }
      });
    QString profileText;
    if(selectedAccounts.size() == 1) {
      profileText = tr("View Profile");
    } else {
      profileText = tr("View Profiles");
    }
    profileAction->setText(profileText);
    contextMenu->addAction(profileAction);
    auto flattenAction = new FunctionalAction(contextMenu.get());
    flattenAction->SetFunction(
      [&] {
        QMessageBox::StandardButton selection = QMessageBox::warning(this,
          tr("Confirm"),
          tr("Are you sure you want to flatten all selected positions?"),
          QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
        if(selection == QMessageBox::Cancel) {
          return;
        }
        for(auto& position : positions) {
          FlattenPosition(position, *m_userProfile);
        }
      });
    QString flattenText;
    if(positions.size() == 1) {
      flattenText = tr("Flatten Position");
    } else {
      flattenText = tr("Flatten Selected Positions");
    }
    flattenAction->setText(flattenText);
    if(!positions.empty()) {
      contextMenu->addAction(flattenAction);
    }
  }
  auto hasRows = m_ui->m_portfolioTableView->model()->rowCount() > 0;
  auto hasSelection = !selectedRows.empty();
  auto exportAllAction = new FunctionalAction(contextMenu.get());
  exportAllAction->SetFunction(
    [&] {
      auto path = QFileDialog::getSaveFileName(this,
        tr("Select file to export to."), QStandardPaths::writableLocation(
        QStandardPaths::DocumentsLocation) + "/portfolio.csv", "CSV (*.csv)");
      if(path.isNull()) {
        return;
      }
      ofstream exportFile(path.toStdString());
      ExportModelAsCsv(*m_userProfile, *m_ui->m_portfolioTableView->model(),
        exportFile);
    });
  exportAllAction->setEnabled(hasRows);
  exportAllAction->setText(tr("Export To File"));
  contextMenu->addAction(exportAllAction);
  auto selectedAction = dynamic_cast<FunctionalAction*>(contextMenu->exec(
    static_cast<QWidget*>(sender())->mapToGlobal(position)));
  if(selectedAction != nullptr) {
    selectedAction->Execute();
  }
}
