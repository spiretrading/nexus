#ifndef SPIRE_USER_PROFILE_HPP
#define SPIRE_USER_PROFILE_HPP
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <boost/date_time/local_time/tz_database.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Nexus/TelemetryService/TelemetryClientBox.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/Catalog/CatalogSettings.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindowSettings.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindowSettings.hpp"
#include "Spire/RiskTimer/RiskTimerProperties.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Ui/SecurityBox.hpp"

namespace Spire {

  /** The type of model used to store the list of recently closed windows. */
  using RecentlyClosedWindowListModel =
    ListModel<std::shared_ptr<LegacyUI::WindowSettings>>;

  /** Stores the user's preferences and application data. */
  class UserProfile {
    public:

      /**
       * Constructs a UserProfile.
       * @param username The username.
       * @param isAdministrator Whether the account is a system administrator.
       * @param isManager Whether the account manages at least one trading
       *        group.
       * @param countryDatabase Stores the database of all countries.
       * @param timeZoneDatabase Stores the database of all time zones.
       * @param currencyDatabase Stores the database of all currencies.
       * @param exchangeRates The list of ExchangeRates to use.
       * @param marketDatabase Stores the database of all markets.
       * @param destinationDatabase Stores the database of all destinations.
       * @param entitlementDatabase Stores the database of market data
       *        entitlements.
       * @param additionalTagDatabase Stores the database of additional tags.
       * @param book_view_properties Initializes the display properties of the
       *        BookViewWindow.
       * @param time_and_sales_properties Initializes the time and sales
       *        properties.
       * @param serviceClients The set of clients connected to Spire services.
       * @param telemetryClient The client used to submit telemetry data.
       */
      UserProfile(const std::string& username, bool isAdministrator,
        bool isManager, const Nexus::CountryDatabase& countryDatabase,
        const boost::local_time::tz_database& timeZoneDatabase,
        const Nexus::CurrencyDatabase& currencyDatabase,
        const std::vector<Nexus::ExchangeRate>& exchangeRates,
        const Nexus::MarketDatabase& marketDatabase,
        const Nexus::DestinationDatabase& destinationDatabase,
        const Nexus::MarketDataService::EntitlementDatabase&
          entitlementDatabase,
        const AdditionalTagDatabase& additionalTagDatabase,
        BookViewProperties book_view_properties,
        TimeAndSalesProperties time_and_sales_properties,
        Nexus::ServiceClientsBox serviceClients,
        Nexus::TelemetryService::TelemetryClientBox telemetryClient);

      ~UserProfile();

      /** Returns the username. */
      const std::string& GetUsername() const;

      /** Returns <code>true</code> iff the account is an administrator. */
      bool IsAdministrator() const;

      /**
       * Returns <code>true</code> iff the account manages at least one trading
       * group.
       */
      bool IsManager() const;

      /** Returns the CountryDatabase. */
      const Nexus::CountryDatabase& GetCountryDatabase() const;

      /** Returns the time zone database. */
      const boost::local_time::tz_database& GetTimeZoneDatabase() const;

      /** Returns the CurrencyDatabase. */
      const Nexus::CurrencyDatabase& GetCurrencyDatabase() const;

      /** Returns the ExchangeRates. */
      const Nexus::ExchangeRateTable& GetExchangeRates() const;

      /** Returns the MarketDatabase. */
      const Nexus::MarketDatabase& GetMarketDatabase() const;

      /** Returns the DestinationDatabase. */
      const Nexus::DestinationDatabase& GetDestinationDatabase() const;

      /** Returns the EntitlementDatabase. */
      const Nexus::MarketDataService::EntitlementDatabase&
        GetEntitlementDatabase() const;

      /** Returns the set of clients connected to Spire services. */
      Nexus::ServiceClientsBox& GetServiceClients() const;

      /** Returns the telemetry client. */
      Nexus::TelemetryService::TelemetryClientBox& GetTelemetryClient() const;

      /** Creates the profile path. */
      void CreateProfilePath() const;

      /** Returns the path to this user's profile directory. */
      const std::filesystem::path& GetProfilePath() const;

      /** Returns the list of recently closed windows. */
      const std::shared_ptr<RecentlyClosedWindowListModel>&
        GetRecentlyClosedWindows() const;

      /** Returns the model used to query securities. */
      const std::shared_ptr<SecurityInfoQueryModel>&
        GetSecurityInfoQueryModel() const;

      /** Returns the BlotterSettings. */
      const BlotterSettings& GetBlotterSettings() const;

      /** Returns the BlotterSettings. */
      BlotterSettings& GetBlotterSettings();

      /** Returns the SavedDashboards. */
      SavedDashboards& GetSavedDashboards();

      /** Returns the SavedDashboards. */
      const SavedDashboards& GetSavedDashboards() const;

      /** Returns the database of additional tags. */
      const AdditionalTagDatabase& GetAdditionalTagDatabase() const;

      /** Returns the key bindings. */
      const std::shared_ptr<KeyBindingsModel>& GetKeyBindings() const;

      /** Returns the CatalogSettings. */
      const CatalogSettings& GetCatalogSettings() const;

      /** Returns the CatalogSettings. */
      CatalogSettings& GetCatalogSettings();

      /** Returns the CanvasTypeRegistry. */
      const CanvasTypeRegistry& GetCanvasTypeRegistry() const;

      /** Returns the CanvasTypeRegistry. */
      CanvasTypeRegistry& GetCanvasTypeRegistry();

      /** Returns the default OrderImbalanceIndicatorProperties. */
      const OrderImbalanceIndicatorProperties&
        GetDefaultOrderImbalanceIndicatorProperties() const;

      /**
       * Sets the default OrderImbalanceIndicatorProperties.
       * @param properties The OrderImbalanceIndicatorProperties to use as the
       *        defaults.
       */
      void SetDefaultOrderImbalanceIndicatorProperties(
        const OrderImbalanceIndicatorProperties& properties);

      /** Returns the initial OrderImbalanceIndicatorWindowSettings. */
      const boost::optional<OrderImbalanceIndicatorWindowSettings>&
        GetInitialOrderImbalanceIndicatorWindowSettings() const;

      /** Sets the initial OrderImbalanceIndicatorWindowSettings. */
      void SetInitialOrderImbalanceIndicatorWindowSettings(
        const OrderImbalanceIndicatorWindowSettings& settings);

      /** Returns the BookViewPropertiesWindowFactory. */
      const std::shared_ptr<BookViewPropertiesWindowFactory>&
        GetBookViewPropertiesWindowFactory() const;

      /** Returns the BookViewModelBuilder. */
      const BookViewWindow::ModelBuilder& GetBookViewModelBuilder() const;

      /** Returns the RiskTimerProperties. */
      const RiskTimerProperties& GetRiskTimerProperties() const;

      /** Returns the RiskTimerProperties. */
      RiskTimerProperties& GetRiskTimerProperties();

      /** Returns the TimeAndSalesPropertiesWindowFactory. */
      const std::shared_ptr<TimeAndSalesPropertiesWindowFactory>&
        GetTimeAndSalesPropertiesWindowFactory() const;

      /** Returns the TimeAndSalesModelBuilder. */
      const TimeAndSalesWindow::ModelBuilder&
        GetTimeAndSalesModelBuilder() const;

      /** Returns the default PortfolioViewerProperties. */
      const PortfolioViewerProperties&
        GetDefaultPortfolioViewerProperties() const;

      /**
       * Sets the default PortfolioViewerProperties.
       * @param properties The PortfolioViewerProperties to use as defaults.
       */
      void SetDefaultPortfolioViewerProperties(
        const PortfolioViewerProperties& properties);

      /** Returns the initial PortfolioViewerWindowSettings. */
      const boost::optional<PortfolioViewerWindowSettings>&
        GetInitialPortfolioViewerWindowSettings() const;

      /** Sets the initial PortfolioViewerWindowSettings. */
      void SetInitialPortfolioViewerWindowSettings(
        const PortfolioViewerWindowSettings& settings);

    private:
      std::string m_username;
      bool m_isAdministrator;
      bool m_isManager;
      Nexus::CountryDatabase m_countryDatabase;
      boost::local_time::tz_database m_timeZoneDatabase;
      Nexus::CurrencyDatabase m_currencyDatabase;
      Nexus::ExchangeRateTable m_exchangeRates;
      Nexus::MarketDatabase m_marketDatabase;
      Nexus::DestinationDatabase m_destinationDatabase;
      Nexus::MarketDataService::EntitlementDatabase m_entitlementDatabase;
      mutable Nexus::ServiceClientsBox m_serviceClients;
      mutable Nexus::TelemetryService::TelemetryClientBox m_telemetryClient;
      std::filesystem::path m_profilePath;
      std::shared_ptr<RecentlyClosedWindowListModel> m_recentlyClosedWindows;
      std::shared_ptr<SecurityInfoQueryModel> m_security_info_query_model;
      SavedDashboards m_savedDashboards;
      OrderImbalanceIndicatorProperties
        m_defaultOrderImbalanceIndicatorProperties;
      std::shared_ptr<BookViewPropertiesWindowFactory>
        m_book_view_properties_window_factory;
      BookViewWindow::ModelBuilder m_book_view_model_builder;
      RiskTimerProperties m_riskTimerProperties;
      std::shared_ptr<TimeAndSalesPropertiesWindowFactory>
        m_time_and_sales_properties_window_factory;
      TimeAndSalesWindow::ModelBuilder m_time_and_sales_model_builder;
      PortfolioViewerProperties m_defaultPortfolioViewerProperties;
      CatalogSettings m_catalogSettings;
      AdditionalTagDatabase m_additionalTagDatabase;
      std::shared_ptr<KeyBindingsModel> m_keyBindings;
      CanvasTypeRegistry m_typeRegistry;
      std::unique_ptr<BlotterSettings> m_blotterSettings;
      boost::optional<OrderImbalanceIndicatorWindowSettings>
        m_initialOrderImbalanceIndicatorWindowSettings;
      boost::optional<PortfolioViewerWindowSettings>
        m_initialPortfolioViewerWindowSettings;
  };

  /** Returns the path to the folder containing all user profiles. */
  std::filesystem::path get_profile_path();

  /**
   * Returns the path to the user's profile folder.
   * @param username The username to get the profile path for.
   */
  std::filesystem::path get_profile_path(const std::string& username);

  /**
   * Returns the default order quantity to display to a user.
   * @param userProfile The profile of the user to display the default order
   *        quantity to.
   * @param security The security that the user is entering a quantity for.
   * @return The default quantity to display.
   */
  Nexus::Quantity get_default_order_quantity(const UserProfile& userProfile,
    const Nexus::Security& security, Nexus::Side side);
}

#endif
