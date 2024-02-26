#include "Spire/LegacyUI/Toolbar.hpp"
#include <boost/lexical_cast.hpp>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include "Spire/AccountViewer/AccountViewWindow.hpp"
#include "Spire/AccountViewer/TraderProfileWindow.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Catalog/CatalogWindow.hpp"
#include "Spire/Charting/ChartWindow.hpp"
#include "Spire/Dashboard/DashboardWindow.hpp"
#include "Spire/Dashboard/DashboardModelSchema.hpp"
#include "Spire/KeyBindings/SimplifiedKeyBindingsDialog.hpp"
#include "Spire/KeyBindings/TaskKeyBindingsDialog.hpp"
#include "Spire/LegacyUI/CanvasWindow.hpp"
#include "Spire/LegacyUI/FunctionalAction.hpp"
#include "Spire/LegacyUI/ImportExportDialog.hpp"
#include "Spire/LegacyUI/LineInputDialog.hpp"
#include "Spire/LegacyUI/ToolbarWindowSettings.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "ui_Toolbar.h"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  class RecentlyClosedAction : public QAction {
    public:
      RecentlyClosedAction(WindowSettings* window, const QString& text,
        QObject* parent)
          : QAction(text, parent),
            m_window(window) {}

      virtual ~RecentlyClosedAction() {}

      WindowSettings& GetWindow() {
        return *m_window;
      }

    private:
      WindowSettings* m_window;
  };
}

Toolbar::Toolbar(Ref<UserProfile> userProfile, QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_Toolbar>()),
      m_userProfile(userProfile.Get()) {
  m_ui->setupUi(this);
  if(!m_userProfile->IsManager()) {
    m_ui->m_administrationGroup->hide();
  }
  setWindowTitle(QString::fromStdString(
    "Spire - Logged in as " + m_userProfile->GetUsername()));
  m_recentlyClosedMenu = new QMenu(this);
  m_ui->m_recentlyClosedButton->setMenu(m_recentlyClosedMenu);
  connect(m_ui->m_recentlyClosedButton, &QToolButton::pressed, this,
    &Toolbar::OnRecentlyClosedButtonPressed);
  connect(m_recentlyClosedMenu, &QMenu::triggered, this,
    &Toolbar::OnRecentlyClosedAction);
  m_minimizeAllAction = new QAction(this);
  m_minimizeAllAction->setText(tr("Minimize All"));
  connect(m_minimizeAllAction, &QAction::triggered, this,
    &Toolbar::OnMinimizeAllAction);
  m_ui->m_windowManagerButton->addAction(m_minimizeAllAction);
  m_restoreAllAction = new QAction(this);
  m_restoreAllAction->setText(tr("Restore All"));
  connect(m_restoreAllAction, &QAction::triggered, this,
    &Toolbar::OnRestoreAllAction);
  m_ui->m_windowManagerButton->addAction(m_restoreAllAction);
  m_importExportAction = new QAction(this);
  m_importExportAction->setText(tr("Import/Export Settings"));
  connect(m_importExportAction, &QAction::triggered, this,
    &Toolbar::OnImportExportAction);
  m_ui->m_windowManagerButton->addAction(m_importExportAction);
  m_newCanvasAction = new QAction(this);
  m_newCanvasAction->setText(tr("New Canvas"));
  m_newCanvasAction->setToolTip(tr("Opens a new Canvas."));
  m_newCanvasAction->setShortcut(QKeySequence::New);
  addAction(m_newCanvasAction);
  connect(m_newCanvasAction, &QAction::triggered, this,
    &Toolbar::OnNewCanvasAction);
  connect(m_ui->m_newCanvasButton, &QToolButton::clicked, m_newCanvasAction,
    &QAction::trigger);
  m_profileAction = new QAction(this);
  m_profileAction->setText(tr("View Profile"));
  m_profileAction->setToolTip(tr("Views the account's profile."));
  m_profileAction->setShortcut(QKeySequence::Open);
  addAction(m_profileAction);
  connect(m_profileAction, &QAction::triggered, this,
    &Toolbar::OnProfileAction);
  connect(m_ui->m_profileButton, &QToolButton::clicked, m_profileAction,
    &QAction::trigger);
  m_keyBindingsAction = new QAction(this);
  m_keyBindingsAction->setText(tr("Setup Key Bindings"));
  m_keyBindingsAction->setToolTip(tr("Sets up keyboard buttons."));
  addAction(m_keyBindingsAction);
  connect(m_keyBindingsAction, &QAction::triggered, this,
    &Toolbar::OnKeyBindingsAction);
  connect(m_ui->m_keyBindingsButton, &QToolButton::clicked, m_keyBindingsAction,
    &QAction::trigger);
  m_logoutAction = new QAction(this);
  m_logoutAction->setText(tr("Logout"));
  m_logoutAction->setToolTip(tr("Logs out of Spire."));
  m_logoutAction->setShortcut(QKeySequence::Close);
  addAction(m_logoutAction);
  connect(m_logoutAction, &QAction::triggered, this, &Toolbar::OnLogoutAction);
  connect(m_ui->m_logoutButton, &QToolButton::clicked, m_logoutAction,
    &QAction::trigger);
  m_openBookViewAction = new QAction(this);
  m_openBookViewAction->setText(tr("Book View"));
  m_openBookViewAction->setToolTip(tr("Opens a Book View window."));
  m_openBookViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
  addAction(m_openBookViewAction);
  connect(m_openBookViewAction, &QAction::triggered, this,
    &Toolbar::OnOpenBookViewAction);
  connect(m_ui->m_openBookViewButton, &QToolButton::clicked,
    m_openBookViewAction, &QAction::trigger);
  m_openChartWindowAction = new QAction(this);
  m_openChartWindowAction->setText(tr("Chart Window"));
  m_openChartWindowAction->setToolTip(tr("Opens a chart window."));
  addAction(m_openChartWindowAction);
  connect(m_openChartWindowAction, &QAction::triggered, this,
    &Toolbar::OnOpenChartWindowAction);
  connect(m_ui->m_openChartButton, &QToolButton::clicked,
    m_openChartWindowAction, &QAction::trigger);
  m_openDashboardWindowAction = new QAction(this);
  m_openDashboardWindowAction->setText(tr("Dashboard Window"));
  m_openDashboardWindowAction->setToolTip(tr("Opens a dashboard window."));
  addAction(m_openDashboardWindowAction);
  connect(m_openDashboardWindowAction, &QAction::triggered, this,
    &Toolbar::OnOpenDashboardWindowAction);
  connect(m_ui->m_openDashboardButton, &QToolButton::clicked,
    m_openDashboardWindowAction, &QAction::trigger);
  m_openTimeAndSalesAction = new QAction(this);
  m_openTimeAndSalesAction->setText(tr("Time and Sales"));
  m_openTimeAndSalesAction->setToolTip(tr("Opens a Time and Sales window."));
  m_openTimeAndSalesAction->setShortcut(
    QKeySequence(Qt::CTRL + Qt::Key_T));
  addAction(m_openTimeAndSalesAction);
  connect(m_openTimeAndSalesAction, &QAction::triggered, this,
    &Toolbar::OnOpenTimeAndSalesAction);
  connect(m_ui->m_openTimeAndSalesButton, &QToolButton::clicked,
    m_openTimeAndSalesAction, &QAction::trigger);
  m_openOrderImbalanceIndicatorAction = new QAction(this);
  m_openOrderImbalanceIndicatorAction->setText(tr("Order Imbalance Indicator"));
  m_openOrderImbalanceIndicatorAction->setToolTip(
    tr("Opens an Order Imbalance Indicator window."));
  m_openOrderImbalanceIndicatorAction->setShortcut(
    QKeySequence(Qt::CTRL + Qt::Key_I));
  addAction(m_openOrderImbalanceIndicatorAction);
  connect(m_openOrderImbalanceIndicatorAction, &QAction::triggered, this,
    &Toolbar::OnOpenOrderImbalanceIndicatorAction);
  connect(m_ui->m_openOrderImbalanceIndicatorButton, &QToolButton::clicked,
    m_openOrderImbalanceIndicatorAction, &QAction::trigger);
  m_blotterMenu = new QMenu(this);
  m_ui->m_openBlotterButton->setMenu(m_blotterMenu);
  connect(m_ui->m_openBlotterButton, &QToolButton::pressed,
    m_ui->m_openBlotterButton, &QToolButton::showMenu);
  m_newBlotterAction = new QAction(this);
  m_newBlotterAction->setText(tr("New"));
  m_newBlotterAction->setToolTip(tr("Creates a new blotter."));
  m_blotterMenu->addAction(m_newBlotterAction);
  connect(m_newBlotterAction, &QAction::triggered, this,
    &Toolbar::OnNewBlotterAction);
  m_accountViewAction = new QAction(this);
  m_accountViewAction->setText(tr("Account Viewer"));
  m_accountViewAction->setToolTip(
    tr("Displays the traders managed by this account."));
  m_openTimeAndSalesAction->setShortcut(
    QKeySequence(Qt::CTRL + Qt::Key_A));
  addAction(m_accountViewAction);
  connect(m_accountViewAction, &QAction::triggered, this,
    &Toolbar::OnAccountViewAction);
  connect(m_ui->m_accountsButton, &QToolButton::clicked, m_accountViewAction,
    &QAction::trigger);
  m_openPortfolioViewerAction = new QAction(this);
  m_openPortfolioViewerAction->setText(tr("View Portfolios"));
  m_openPortfolioViewerAction->setToolTip(
    tr("Opens the portfolio of all managed accounts."));
  addAction(m_openPortfolioViewerAction);
  connect(m_openPortfolioViewerAction, &QAction::triggered, this,
    &Toolbar::OnOpenPortfolioViewerAction);
  connect(m_ui->m_openPortfolioViewerButton, &QToolButton::clicked,
    m_openPortfolioViewerAction, &QAction::trigger);
  m_blotterMenu->addSeparator();
  BlotterSettings& blotterSettings = m_userProfile->GetBlotterSettings();
  m_blotterAddedConnection = blotterSettings.ConnectBlotterAddedSignal(
    std::bind(&Toolbar::OnBlotterAdded, this, std::placeholders::_1));
  m_blotterRemovedConnection = blotterSettings.ConnectBlotterRemovedSignal(
    std::bind(&Toolbar::OnBlotterRemoved, this, std::placeholders::_1));
  for(const auto& blotter : blotterSettings.GetAllBlotters()) {
    OnBlotterAdded(*blotter);
  }
  auto h = height();
  adjustSize();
  setFixedSize(width(), h);
}

unique_ptr<WindowSettings> Toolbar::GetWindowSettings() const {
  auto settings = std::make_unique<ToolbarWindowSettings>(*this);
  return std::move(settings);
}

void Toolbar::closeEvent(QCloseEvent* event) {
  WindowSettings::Save(*m_userProfile);
  for(const auto& widget : QApplication::topLevelWidgets()) {
    if(widget != this) {
      widget->close();
    }
  }
  event->accept();
  QApplication::quit();
}

void Toolbar::OnBlotterAdded(BlotterModel& blotter) {
  auto openBlotterAction = new QAction(this);
  auto entryName = QString::fromStdString(blotter.GetName());
  openBlotterAction->setText(entryName);
  m_blotterMenu->addAction(openBlotterAction);
  m_actionToBlotter.insert(make_pair(openBlotterAction, &blotter));
  connect(openBlotterAction, &QAction::triggered, this,
    &Toolbar::OnOpenBlotterAction);
}

void Toolbar::OnBlotterRemoved(BlotterModel& blotter) {
  for(auto i = m_actionToBlotter.begin();
      i != m_actionToBlotter.end(); ++i) {
    if(i->second == &blotter) {
      m_blotterMenu->removeAction(i->first);
      delete i->first;
      m_actionToBlotter.erase(i);
      return;
    }
  }
}

void Toolbar::OnMinimizeAllAction() {
  for(const auto& widget : QApplication::topLevelWidgets()) {
    widget->setWindowState(Qt::WindowMinimized);
  }
}

void Toolbar::OnRestoreAllAction() {
  for(const auto& widget : QApplication::topLevelWidgets()) {
    widget->setWindowState(Qt::WindowActive);
  }
}

void Toolbar::OnImportExportAction() {
  auto dialog = ImportExportDialog(Ref(*m_userProfile));
  dialog.exec();
}

void Toolbar::OnRecentlyClosedButtonPressed() {
/*
  m_recentlyClosedMenu->clear();
  const auto& recentlyClosedWindows = m_userProfile->GetRecentlyClosedWindows();
  if(recentlyClosedWindows.empty()) {
    auto action = new QAction(tr("Empty"), m_recentlyClosedMenu);
    action->setEnabled(false);
    m_recentlyClosedMenu->addAction(action);
  } else {
    auto count = 1;
    for(const auto& settings : recentlyClosedWindows) {
      auto action = new RecentlyClosedAction(settings.get(),
        QString::fromStdString(lexical_cast<string>(count) + "  -  " +
        settings->GetName()), m_recentlyClosedMenu);
      m_recentlyClosedMenu->addAction(action);
      ++count;
    }
  }
  m_ui->m_recentlyClosedButton->showMenu();
*/
}

void Toolbar::OnRecentlyClosedAction(QAction* action) {
/*
  auto closedAction = static_cast<RecentlyClosedAction*>(action);
  auto& window = closedAction->GetWindow();
  auto widget = window.Reopen(Ref(*m_userProfile));
  widget->show();
  m_userProfile->RemoveRecentlyClosedWindow(window);
*/
}

void Toolbar::OnNewCanvasAction() {
  auto canvas = new CanvasWindow(Ref(*m_userProfile));
  canvas->setAttribute(Qt::WA_DeleteOnClose);
  canvas->show();
}

void Toolbar::OnProfileAction() {
  auto profileWindow = new TraderProfileWindow(Ref(*m_userProfile));
  profileWindow->setAttribute(Qt::WA_DeleteOnClose);
  profileWindow->Load(
    m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount());
  profileWindow->show();
}

void Toolbar::OnKeyBindingsAction() {
  auto dialog = new SimplifiedKeyBindingsDialog(Ref(*m_userProfile));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
}

void Toolbar::OnLogoutAction() {
  close();
}

void Toolbar::OnOpenTimeAndSalesAction() {
  auto timeAndSalesWindow = new TimeAndSalesWindow(Ref(*m_userProfile),
    m_userProfile->GetDefaultTimeAndSalesProperties());
  timeAndSalesWindow->setAttribute(Qt::WA_DeleteOnClose);
  timeAndSalesWindow->show();
}

void Toolbar::OnOpenBookViewAction() {
  auto bookViewWindow = new BookViewWindow(Ref(*m_userProfile),
    m_userProfile->GetDefaultBookViewProperties());
  bookViewWindow->setAttribute(Qt::WA_DeleteOnClose);
  bookViewWindow->show();
}

void Toolbar::OnOpenChartWindowAction() {
  auto chartWindow = new ChartWindow(Ref(*m_userProfile));
  chartWindow->setAttribute(Qt::WA_DeleteOnClose);
  chartWindow->show();
}

void Toolbar::OnOpenDashboardWindowAction() {
  auto window = new DashboardWindow{DashboardWindow::GetDefaultName(),
    DashboardModelSchema::GetDefaultSchema(), Ref(*m_userProfile)};
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->show();
}

void Toolbar::OnOpenOrderImbalanceIndicatorAction() {
  auto settings =
    m_userProfile->GetInitialOrderImbalanceIndicatorWindowSettings();
  if(settings.is_initialized()) {
    auto window = settings->Reopen(Ref(*m_userProfile));
    window->show();
  } else {
    auto model = std::make_shared<OrderImbalanceIndicatorModel>(
      Ref(*m_userProfile),
      m_userProfile->GetDefaultOrderImbalanceIndicatorProperties());
    auto orderImbalanceIndicatorWindow = new OrderImbalanceIndicatorWindow(
      Ref(*m_userProfile), model);
    orderImbalanceIndicatorWindow->setAttribute(Qt::WA_DeleteOnClose);
    orderImbalanceIndicatorWindow->show();
  }
}

void Toolbar::OnOpenBlotterAction() {
  auto action = qobject_cast<QAction*>(sender());
  auto model = m_actionToBlotter[action];
  assert(model != nullptr);
  auto& window = BlotterWindow::GetBlotterWindow(Ref(*m_userProfile),
    Ref(*model));
  window.show();
  window.activateWindow();
}

void Toolbar::OnNewBlotterAction() {
  LineInputDialog newBlotterDialog("New Blotter", "Name:", "");
  if(newBlotterDialog.exec() == QDialog::Rejected) {
    return;
  }
  auto blotter = std::make_unique<BlotterModel>(newBlotterDialog.GetInput(),
    m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount(),
    false, Ref(*m_userProfile),
    m_userProfile->GetBlotterSettings().GetDefaultBlotterTaskProperties(),
    m_userProfile->GetBlotterSettings().GetDefaultOrderLogProperties());
  m_userProfile->GetBlotterSettings().AddBlotter(std::move(blotter));
  auto& window = BlotterWindow::GetBlotterWindow(Ref(*m_userProfile),
    Ref(*m_userProfile->GetBlotterSettings().GetAllBlotters().back()));
  window.show();
  window.activateWindow();
}

void Toolbar::OnAccountViewAction() {
  auto accountViewWindow = new AccountViewWindow(Ref(*m_userProfile));
  accountViewWindow->setAttribute(Qt::WA_DeleteOnClose);
  accountViewWindow->show();
}

void Toolbar::OnOpenPortfolioViewerAction() {
  auto settings = m_userProfile->GetInitialPortfolioViewerWindowSettings();
  if(settings.is_initialized()) {
    auto window = settings->Reopen(Ref(*m_userProfile));
    window->show();
  } else {
    auto portfolioViewerWindow = new PortfolioViewerWindow(Ref(*m_userProfile),
      m_userProfile->GetDefaultPortfolioViewerProperties());
    portfolioViewerWindow->setAttribute(Qt::WA_DeleteOnClose);
    portfolioViewerWindow->show();
  }
}
