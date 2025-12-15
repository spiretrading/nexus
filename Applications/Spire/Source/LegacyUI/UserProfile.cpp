#include "Spire/LegacyUI/UserProfile.hpp"
#include <QStandardPaths>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/OpenPositionsModel.hpp"
#include "Spire/BookView/ServiceBookViewModel.hpp"
#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ServiceSecurityInfoQueryModel.hpp"
#include "Spire/TimeAndSales/ServiceTimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  std::shared_ptr<TimeAndSalesModel> time_and_sales_model_builder(
      const Security& security, MarketDataClient client) {
    return std::make_shared<ServiceTimeAndSalesModel>(security, client);
  }

  std::shared_ptr<BookViewModel> book_view_model_builder(
      const Security& security, BlotterSettings& blotter,
      MarketDataClient client) {
    return std::make_shared<ServiceBookViewModel>(security, blotter, client);
  }
}

UserProfile::UserProfile(const std::string& username, bool isAdministrator,
    bool isManager, const std::vector<ExchangeRate>& exchangeRates,
    const EntitlementDatabase& entitlementDatabase,
    const AdditionalTagDatabase& additionalTagDatabase,
    BookViewProperties book_view_properties,
    TimeAndSalesProperties time_and_sales_properties, Clients clients)
BEAM_SUPPRESS_THIS_INITIALIZER()
    : m_username(username),
      m_isAdministrator(isAdministrator),
      m_isManager(isManager),
      m_entitlementDatabase(entitlementDatabase),
      m_clients(std::move(clients)),
      m_profilePath(std::filesystem::path(QStandardPaths::writableLocation(
        QStandardPaths::DataLocation).toStdString()) / "Profiles" / m_username),
      m_recentlyClosedWindows(
        std::make_shared<ArrayListModel<std::shared_ptr<WindowSettings>>>()),
      m_security_info_query_model(
        std::make_shared<ServiceSecurityInfoQueryModel>(
          m_clients.get_market_data_client())),
      m_book_view_properties_window_factory(
        std::make_shared<BookViewPropertiesWindowFactory>(
          std::make_shared<LocalBookViewPropertiesModel>(
            std::move(book_view_properties)))),
      m_book_view_model_builder([=] (const auto& security) {
        return book_view_model_builder(
          security, *m_blotterSettings, m_clients.get_market_data_client());
      }),
      m_time_and_sales_properties_window_factory(
        std::make_shared<TimeAndSalesPropertiesWindowFactory>(
          std::make_shared<LocalTimeAndSalesPropertiesModel>(
            std::move(time_and_sales_properties)))),
      m_time_and_sales_model_builder([=] (const auto& security) {
        return time_and_sales_model_builder(
          security, m_clients.get_market_data_client());
      }),
      m_catalogSettings(m_profilePath / "Catalog", isAdministrator),
      m_additionalTagDatabase(additionalTagDatabase) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  m_keyBindings = load_key_bindings_profile(m_profilePath);
  for(auto& exchangeRate : exchangeRates) {
    m_exchangeRates.add(exchangeRate);
  }
  m_blotterSettings = std::make_unique<BlotterSettings>(Ref(*this));
}

UserProfile::~UserProfile() = default;

const std::string& UserProfile::GetUsername() const {
  return m_username;
}

bool UserProfile::IsAdministrator() const {
  return m_isAdministrator;
}

bool UserProfile::IsManager() const {
  return m_isAdministrator || m_isManager;
}

const ExchangeRateTable& UserProfile::GetExchangeRates() const {
  return m_exchangeRates;
}

const EntitlementDatabase& UserProfile::GetEntitlementDatabase() const {
  return m_entitlementDatabase;
}

Clients& UserProfile::GetClients() const {
  return m_clients;
}

void UserProfile::CreateProfilePath() const {
  if(std::filesystem::is_directory(m_profilePath)) {
    return;
  }
  if(std::filesystem::exists(m_profilePath)) {
    std::filesystem::remove(m_profilePath);
  }
  std::filesystem::create_directories(m_profilePath);
}

const std::filesystem::path& UserProfile::GetProfilePath() const {
  return m_profilePath;
}

const std::shared_ptr<RecentlyClosedWindowListModel>&
    UserProfile::GetRecentlyClosedWindows() const {
  return m_recentlyClosedWindows;
}

const std::shared_ptr<SecurityInfoQueryModel>&
    UserProfile::GetSecurityInfoQueryModel() const {
  return m_security_info_query_model;
}

const BlotterSettings& UserProfile::GetBlotterSettings() const {
  return *m_blotterSettings;
}

BlotterSettings& UserProfile::GetBlotterSettings() {
  return *m_blotterSettings;
}

SavedDashboards& UserProfile::GetSavedDashboards() {
  return m_savedDashboards;
}

const SavedDashboards& UserProfile::GetSavedDashboards() const {
  return m_savedDashboards;
}

const AdditionalTagDatabase& UserProfile::GetAdditionalTagDatabase() const {
  return m_additionalTagDatabase;
}

const std::shared_ptr<KeyBindingsModel>& UserProfile::GetKeyBindings() const {
  return m_keyBindings;
}

const CatalogSettings& UserProfile::GetCatalogSettings() const {
  return m_catalogSettings;
}

CatalogSettings& UserProfile::GetCatalogSettings() {
  return m_catalogSettings;
}

const CanvasTypeRegistry& UserProfile::GetCanvasTypeRegistry() const {
  return m_typeRegistry;
}

CanvasTypeRegistry& UserProfile::GetCanvasTypeRegistry() {
  return m_typeRegistry;
}

const OrderImbalanceIndicatorProperties&
    UserProfile::GetDefaultOrderImbalanceIndicatorProperties() const {
  return m_defaultOrderImbalanceIndicatorProperties;
}

void UserProfile::SetDefaultOrderImbalanceIndicatorProperties(
    const OrderImbalanceIndicatorProperties& properties) {
  m_defaultOrderImbalanceIndicatorProperties = properties;
}

const optional<OrderImbalanceIndicatorWindowSettings>&
    UserProfile::GetInitialOrderImbalanceIndicatorWindowSettings() const {
  return m_initialOrderImbalanceIndicatorWindowSettings;
}

void UserProfile::SetInitialOrderImbalanceIndicatorWindowSettings(
    const OrderImbalanceIndicatorWindowSettings& settings) {
  m_initialOrderImbalanceIndicatorWindowSettings = settings;
}

const std::shared_ptr<BookViewPropertiesWindowFactory>&
    UserProfile::GetBookViewPropertiesWindowFactory() const {
  return m_book_view_properties_window_factory;
}

const BookViewWindow::ModelBuilder&
    UserProfile::GetBookViewModelBuilder() const {
  return m_book_view_model_builder;
}

const RiskTimerProperties& UserProfile::GetRiskTimerProperties() const {
  return m_riskTimerProperties;
}

RiskTimerProperties& UserProfile::GetRiskTimerProperties() {
  return m_riskTimerProperties;
}

const std::shared_ptr<TimeAndSalesPropertiesWindowFactory>&
    UserProfile::GetTimeAndSalesPropertiesWindowFactory() const {
  return m_time_and_sales_properties_window_factory;
}

const TimeAndSalesWindow::ModelBuilder&
    UserProfile::GetTimeAndSalesModelBuilder() const {
  return m_time_and_sales_model_builder;
}

const PortfolioViewerProperties&
    UserProfile::GetDefaultPortfolioViewerProperties() const {
  return m_defaultPortfolioViewerProperties;
}

void UserProfile::SetDefaultPortfolioViewerProperties(
    const PortfolioViewerProperties& properties) {
  m_defaultPortfolioViewerProperties = properties;
}

const optional<PortfolioViewerWindowSettings>&
    UserProfile::GetInitialPortfolioViewerWindowSettings() const {
  return m_initialPortfolioViewerWindowSettings;
}

void UserProfile::SetInitialPortfolioViewerWindowSettings(
    const PortfolioViewerWindowSettings& settings) {
  m_initialPortfolioViewerWindowSettings = settings;
}

std::filesystem::path Spire::get_profile_path() {
  return std::filesystem::weakly_canonical(QStandardPaths::writableLocation(
    QStandardPaths::DataLocation).toStdString()) / "Profiles";
}

std::filesystem::path Spire::get_profile_path(const std::string& username) {
  return get_profile_path() / username;
}

Quantity Spire::get_default_order_quantity(const UserProfile& userProfile,
    const Security& security, Side side) {
  auto position = [&] {
    auto& blotter = userProfile.GetBlotterSettings().GetActiveBlotter();
    if(auto position = blotter.GetOpenPositionsModel().GetOpenPosition(
        security)) {
      return position->m_inventory.m_position.m_quantity;
    }
    return Quantity(0);
  }();
  return get_default_order_quantity(
    *userProfile.GetKeyBindings()->get_interactions_key_bindings(security),
    security, position, side);
}
