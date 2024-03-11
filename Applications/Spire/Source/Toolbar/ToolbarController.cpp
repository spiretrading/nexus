#include "Spire/Toolbar/ToolbarController.hpp"
#include <vector>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/AccountViewer/AccountViewWindow.hpp"
#include "Spire/AccountViewer/TraderProfileWindow.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Charting/ChartWindow.hpp"
#include "Spire/Dashboard/DashboardWindow.hpp"
#include "Spire/Dashboard/DashboardModelSchema.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/LegacyUI/CanvasWindow.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  std::vector<QWidget*> load_default_layout(
      UserProfile& user_profile, ToolbarWindow& toolbar_window) {
    auto instantiate_security_windows = true;
    auto next_position = QPoint(0, 0);
    auto next_height = 0;
    auto resolution = QGuiApplication::primaryScreen()->availableGeometry();
    auto securities = std::vector<Security>();
    auto& market_entry = user_profile.GetMarketDatabase().FromCode("XTSE");
    securities.push_back(
      Security("RY", market_entry.m_code, market_entry.m_countryCode));
    securities.push_back(
      Security("XIU", market_entry.m_code, market_entry.m_countryCode));
    securities.push_back(
      Security("ABX", market_entry.m_code, market_entry.m_countryCode));
    securities.push_back(
      Security("SU", market_entry.m_code, market_entry.m_countryCode));
    securities.push_back(
      Security("BCE", market_entry.m_code, market_entry.m_countryCode));
    auto index = std::size_t(0);
    auto windows = std::vector<QWidget*>();
    while(instantiate_security_windows && index < securities.size()) {
      auto width = 0;
      auto book_view_window = new BookViewWindow(
        Ref(user_profile), user_profile.GetDefaultBookViewProperties(), "");
      auto time_and_sales_window = new TimeAndSalesWindow(
        Ref(user_profile), user_profile.GetDefaultTimeAndSalesProperties(), "");
      book_view_window->Link(*time_and_sales_window);
      time_and_sales_window->Link(*book_view_window);
      book_view_window->move(next_position);
      book_view_window->show();
      next_position.rx() += book_view_window->frameSize().width();
      width += book_view_window->frameSize().width();
      next_height = book_view_window->frameSize().height();
      time_and_sales_window->resize(150, book_view_window->height());
      time_and_sales_window->move(next_position);
      time_and_sales_window->show();
      book_view_window->DisplaySecurity(securities[index]);
      next_position.rx() += time_and_sales_window->frameSize().width();
      width += time_and_sales_window->frameSize().width();
      windows.push_back(book_view_window);
      windows.push_back(time_and_sales_window);
      instantiate_security_windows = index < securities.size() &&
        (next_position.x() + width < resolution.width());
      ++index;
    }
    next_position.setX(0);
    next_position.setY(next_height);
    toolbar_window.move(next_position);
    toolbar_window.show();
    next_position.ry() += toolbar_window.frameSize().height();
    auto& global_blotter = BlotterWindow::GetBlotterWindow(Ref(user_profile),
      Ref(user_profile.GetBlotterSettings().GetConsolidatedBlotter()));
    global_blotter.move(next_position);
    global_blotter.show();
    global_blotter.resize(global_blotter.width(),
      resolution.height() - next_position.y() -
        (global_blotter.frameSize().height() - global_blotter.size().height()));
    windows.push_back(&global_blotter);
    return windows;
  }
}

ToolbarController::ToolbarController(Ref<UserProfile> user_profile)
  : m_user_profile(user_profile.Get()) {}

ToolbarController::~ToolbarController() {
  close();
}

void ToolbarController::open() {
  if(m_toolbar_window) {
    return;
  }
  auto window_settings = WindowSettings::Load(*m_user_profile);
  for(auto i = window_settings.begin(); i != window_settings.end(); ++i) {
    auto& settings = **i;
    if(auto toolbar_settings =
        dynamic_cast<const ToolbarWindowSettings*>(&settings)) {
      m_toolbar_window =
        std::unique_ptr<ToolbarWindow>(dynamic_cast<ToolbarWindow*>(
          toolbar_settings->Reopen(Ref(*m_user_profile))));
      window_settings.erase(i);
      break;
    }
  }
  if(!m_toolbar_window) {
    m_toolbar_window =
      std::unique_ptr<ToolbarWindow>(static_cast<ToolbarWindow*>(
        ToolbarWindowSettings().Reopen(Ref(*m_user_profile))));
  }
  m_pinned_blotters = m_toolbar_window->get_pinned_blotters();
  m_blotter_added_connection =
    m_user_profile->GetBlotterSettings().ConnectBlotterAddedSignal(
      std::bind_front(&ToolbarController::on_blotter_added, this));
  m_blotter_removed_connection =
    m_user_profile->GetBlotterSettings().ConnectBlotterRemovedSignal(
      std::bind_front(&ToolbarController::on_blotter_removed, this));
  auto windows = std::vector<QWidget*>();
  if(!window_settings.empty()) {
    for(auto& settings : window_settings) {
      if(auto window = settings->Reopen(Ref(*m_user_profile))) {
        windows.push_back(window);
      }
    }
  } else {
    windows = load_default_layout(*m_user_profile, *m_toolbar_window);
  }
  for(auto& window : windows) {
    window->show();
  }
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
  m_toolbar_window->connect_import_signal(
    std::bind_front(&ToolbarController::on_import, this));
  m_toolbar_window->connect_export_signal(
    std::bind_front(&ToolbarController::on_export, this));
  m_toolbar_window->connect_new_blotter_signal(
    std::bind_front(&ToolbarController::on_new_blotter, this));
  m_toolbar_window->connect_sign_out_signal(
    std::bind_front(&ToolbarController::on_sign_out, this));
  m_toolbar_window->show();
}

void ToolbarController::close() {
  if(!m_toolbar_window) {
    return;
  }
  WindowSettings::Save(*m_user_profile);
  for(auto& window : QApplication::topLevelWidgets()) {
    if(window != &*m_toolbar_window) {
      window->close();
    }
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

void ToolbarController::open_account_directory_window() {
  auto window = new AccountViewWindow(Ref(*m_user_profile));
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->show();
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
  auto dialog = new KeyBindingsWindow(m_user_profile->GetKeyBindings());
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
  } else if(window == ToolbarWindow::WindowType::ACCOUNT_DIRECTORY) {
    open_account_directory_window();
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

void ToolbarController::on_import(
    UserSettings::Categories categories, const std::filesystem::path& path) {
  import_settings(categories, path, Store(*m_user_profile));
}

void ToolbarController::on_export(
    UserSettings::Categories categories, const std::filesystem::path& path) {
  export_settings(categories, path, *m_user_profile);
}

void ToolbarController::on_new_blotter(const QString& name) {
  auto blotter = std::make_unique<BlotterModel>(name.toStdString(),
    m_user_profile->GetServiceClients().GetServiceLocatorClient().GetAccount(),
    false, Ref(*m_user_profile),
    m_user_profile->GetBlotterSettings().GetDefaultBlotterTaskProperties(),
    m_user_profile->GetBlotterSettings().GetDefaultOrderLogProperties());
  m_user_profile->GetBlotterSettings().AddBlotter(std::move(blotter));
  auto& window = BlotterWindow::GetBlotterWindow(Ref(*m_user_profile),
    Ref(*m_user_profile->GetBlotterSettings().GetAllBlotters().back()));
  window.show();
  window.activateWindow();
}

void ToolbarController::on_blotter_added(BlotterModel& blotter) {
  m_pinned_blotters->push(&blotter);
}

void ToolbarController::on_blotter_removed(BlotterModel& blotter) {
  for(auto i = 0; i != m_pinned_blotters->get_size(); ++i) {
    if(m_pinned_blotters->get(i) == &blotter) {
      m_pinned_blotters->remove(i);
      break;
    }
  }
}

void ToolbarController::on_sign_out() {
  close();
}
