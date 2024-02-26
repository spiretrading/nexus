#include "Spire/Toolbar/ToolbarController.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QApplication>
#include "Spire/AccountViewer/AccountViewWindow.hpp"
#include "Spire/AccountViewer/TraderProfileWindow.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Charting/ChartWindow.hpp"
#include "Spire/Dashboard/DashboardWindow.hpp"
#include "Spire/Dashboard/DashboardModelSchema.hpp"
#include "Spire/KeyBindings/SimplifiedKeyBindingsDialog.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/LegacyUI/CanvasWindow.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;

ToolbarController::ToolbarController(Ref<UserProfile> user_profile)
  : m_user_profile(user_profile.Get()) {}

void ToolbarController::open() {
  if(m_toolbar_window) {
    return;
  }
  auto account =
    m_user_profile->GetServiceClients().GetServiceLocatorClient().GetAccount();
  auto roles = m_user_profile->
    GetServiceClients().GetAdministrationClient().LoadAccountRoles(account);
  auto pinned_blotters = std::make_shared<ArrayListModel<BlotterModel*>>();
  for(auto& blotter : m_user_profile->GetBlotterSettings().GetAllBlotters()) {
    pinned_blotters->push(&*blotter);
  }
  m_toolbar_window = std::make_unique<ToolbarWindow>(
    account, roles, m_user_profile->GetRecentlyClosedWindows(), pinned_blotters);
  m_toolbar_window->connect_open_signal(
    std::bind_front(&ToolbarController::on_open, this));
  m_toolbar_window->connect_reopen_signal(
    std::bind_front(&ToolbarController::on_reopen, this));
  m_toolbar_window->connect_open_blotter_signal(
    std::bind_front(&ToolbarController::on_open_blotter, this));
  m_toolbar_window->connect_minimize_all_signal(
    std::bind_front(&ToolbarController::on_minimize_all, this));
  m_toolbar_window->connect_restore_all_signal(
    std::bind_front(&ToolbarController::on_restore_all, this));
  m_toolbar_window->show();
}

void ToolbarController::close() {
  if(!m_toolbar_window) {
    return;
  }
  m_toolbar_window->close();
  auto window = m_toolbar_window.release();
  window->deleteLater();
  m_toolbar_window = nullptr;
}

void ToolbarController::open_chart_window() {
  auto window = new ChartWindow(Ref(*m_user_profile));
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->show();
}

void ToolbarController::open_book_view_window() {
  auto window = new BookViewWindow(
    Ref(*m_user_profile), m_user_profile->GetDefaultBookViewProperties());
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->show();
}

void ToolbarController::open_time_and_sales_window() {
  auto window = new TimeAndSalesWindow(
    Ref(*m_user_profile), m_user_profile->GetDefaultTimeAndSalesProperties());
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->show();
}

void ToolbarController::open_canvas_window() {
  auto window = new CanvasWindow(Ref(*m_user_profile));
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->show();
}

void ToolbarController::open_watchlist_window() {
  auto window = new DashboardWindow(DashboardWindow::GetDefaultName(),
    DashboardModelSchema::GetDefaultSchema(), Ref(*m_user_profile));
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->show();
}

void ToolbarController::open_order_imbalance_indicator_window() {
  if(auto settings =
      m_user_profile->GetInitialOrderImbalanceIndicatorWindowSettings()) {
    auto window = settings->Reopen(Ref(*m_user_profile));
    window->show();
  } else {
    auto model =
      std::make_shared<OrderImbalanceIndicatorModel>(Ref(*m_user_profile),
        m_user_profile->GetDefaultOrderImbalanceIndicatorProperties());
    auto window =
      new OrderImbalanceIndicatorWindow(Ref(*m_user_profile), model);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
  }
}

void ToolbarController::open_portfolio_window() {
  if(auto settings =
      m_user_profile->GetInitialPortfolioViewerWindowSettings()) {
    auto window = settings->Reopen(Ref(*m_user_profile));
    window->show();
  } else {
    auto window = new PortfolioViewerWindow(Ref(*m_user_profile),
      m_user_profile->GetDefaultPortfolioViewerProperties());
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
  }
}

void ToolbarController::open_key_bindings_window() {
  auto dialog = new SimplifiedKeyBindingsDialog(Ref(*m_user_profile));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
}

void ToolbarController::open_profile_window() {
  auto window = new TraderProfileWindow(Ref(*m_user_profile));
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->Load(
    m_user_profile->GetServiceClients().GetServiceLocatorClient().GetAccount());
  window->show();
}

void ToolbarController::on_open(ToolbarWindow::WindowType window) {
  if(window == ToolbarWindow::WindowType::CHART) {
    open_chart_window();
  } else if(window == ToolbarWindow::WindowType::BOOK_VIEW) {
    open_book_view_window();
  } else if(window == ToolbarWindow::WindowType::TIME_AND_SALES) {
    open_time_and_sales_window();
  } else if(window == ToolbarWindow::WindowType::CANVAS) {
    open_canvas_window();
  } else if(window == ToolbarWindow::WindowType::WATCHLIST) {
    open_watchlist_window();
  } else if(window == ToolbarWindow::WindowType::ORDER_IMBALANCE_INDICATOR) {
    open_order_imbalance_indicator_window();
  } else if(window == ToolbarWindow::WindowType::PORTFOLIO) {
    open_portfolio_window();
  } else if(window == ToolbarWindow::WindowType::KEY_BINDINGS) {
    open_key_bindings_window();
  } else if(window == ToolbarWindow::WindowType::PROFILE) {
    open_profile_window();
  }
}

void ToolbarController::on_reopen(const WindowSettings& settings) {
  auto window = settings.Reopen(Ref(*m_user_profile));
  auto recently_closed_windows = m_user_profile->GetRecentlyClosedWindows();
  for(auto i = 0; i != recently_closed_windows->get_size(); ++i) {
    if(&*recently_closed_windows->get(i) == &settings) {
      recently_closed_windows->remove(i);
      break;
    }
  }
  window->show();
}

void ToolbarController::on_open_blotter(BlotterModel& blotter) {
  auto& window =
    BlotterWindow::GetBlotterWindow(Ref(*m_user_profile), Ref(blotter));
  window.show();
  window.activateWindow();
}

void ToolbarController::on_minimize_all() {
  for(auto& widget : QApplication::topLevelWidgets()) {
    widget->setWindowState(Qt::WindowMinimized);
  }
}

void ToolbarController::on_restore_all() {
  for(auto& widget : QApplication::topLevelWidgets()) {
    widget->setWindowState(Qt::WindowActive);
  }
}
