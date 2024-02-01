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
#include "Nexus/Definitions/RegionMap.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Nexus/TelemetryService/TelemetryClientBox.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/Catalog/CatalogSettings.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/KeyBindings/InteractionsProperties.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindowSettings.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindowSettings.hpp"
#include "Spire/RiskTimer/RiskTimerProperties.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"

namespace Spire {

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
          entitlementDatabase, Nexus::ServiceClientsBox serviceClients,
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
      const std::vector<std::unique_ptr<LegacyUI::WindowSettings>>&
        GetRecentlyClosedWindows() const;

      /**
       * Adds a window to the list of recently closed windows.
       * @param window The recently closed window to add.
       */
      void AddRecentlyClosedWindow(
        std::unique_ptr<LegacyUI::WindowSettings> window);

      /**
       * Removes a window from the list of recently closed windows.
       * @param window The recently closed window to remove.
       */
      void RemoveRecentlyClosedWindow(const LegacyUI::WindowSettings& window);

      /** Returns the BlotterSettings. */
      const BlotterSettings& GetBlotterSettings() const;

      /** Returns the BlotterSettings. */
      BlotterSettings& GetBlotterSettings();

      /** Returns the SavedDashboards. */
      SavedDashboards& GetSavedDashboards();

      /** Returns the SavedDashboards. */
      const SavedDashboards& GetSavedDashboards() const;

      /** Returns the KeyBindings. */
      const KeyBindings& GetKeyBindings() const;

      /** Returns the KeyBindings. */
      KeyBindings& GetKeyBindings();

      /**
       * Sets the KeyBindings.
       * @param keyBindings The new KeyBindings to use for this profile.
       */
      void SetKeyBindings(const KeyBindings& keyBindings);

      /** Returns the CatalogSettings. */
      const CatalogSettings& GetCatalogSettings() const;

      /** Returns the CatalogSettings. */
      CatalogSettings& GetCatalogSettings();

      /** Returns the CanvasTypeRegistry. */
      const CanvasTypeRegistry& GetCanvasTypeRegistry() const;

      /** Returns the CanvasTypeRegistry. */
      CanvasTypeRegistry& GetCanvasTypeRegistry();

      /** Returns the default BookViewProperties. */
      const BookViewProperties& GetDefaultBookViewProperties() const;

      /**
       * Sets the default BookViewProperties.
       * @param properties The BookViewProperties to use as the defaults.
       */
      void SetDefaultBookViewProperties(const BookViewProperties& properties);

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

      /** Returns the RiskTimerProperties. */
      const RiskTimerProperties& GetRiskTimerProperties() const;

      /** Returns the RiskTimerProperties. */
      RiskTimerProperties& GetRiskTimerProperties();

      /** Returns the default TimeAndSalesProperties. */
      const TimeAndSalesProperties& GetDefaultTimeAndSalesProperties() const;

      /**
       * Sets the default TimeAndSalesProperties.
       * @param properties The TimeAndSalesProperties to use as defaults.
       */
      void SetDefaultTimeAndSalesProperties(
        const TimeAndSalesProperties& properties);

      /** Returns the default PortfolioViewerProperties. */
      const PortfolioViewerProperties&
        GetDefaultPortfolioViewerProperties() const;

      /**
       * Sets the default PortfolioViewerProperties.
       * @param properties The PortfolioViewerProperties to use as defaults.
       */
      void SetDefaultPortfolioViewerProperties(
        const PortfolioViewerProperties& properties);

      /** Returns the RegionMap storing the InteractionProperties. */
      Nexus::RegionMap<InteractionsProperties>& GetInteractionProperties();

      /** Returns the RegionMap storing the InteractionProperties. */
      const Nexus::RegionMap<InteractionsProperties>&
        GetInteractionProperties() const;

      /**
       * Returns the default Quantity.
       * @param security The Security to lookup the default Quantity for.
       * @param side The Side to get the default Quantity on.
       * @return The default Quantity used for Tasks.
       */
      Nexus::Quantity GetDefaultQuantity(
        const Nexus::Security& security, Nexus::Side side) const;

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
      std::vector<std::unique_ptr<LegacyUI::WindowSettings>>
        m_recentlyClosedWindows;
      BookViewProperties m_defaultBookViewProperties;
      SavedDashboards m_savedDashboards;
      OrderImbalanceIndicatorProperties
        m_defaultOrderImbalanceIndicatorProperties;
      RiskTimerProperties m_riskTimerProperties;
      TimeAndSalesProperties m_defaultTimeAndSalesProperties;
      PortfolioViewerProperties m_defaultPortfolioViewerProperties;
      Nexus::RegionMap<InteractionsProperties> m_interactionProperties;
      CatalogSettings m_catalogSettings;
      KeyBindings m_keyBindings;
      CanvasTypeRegistry m_typeRegistry;
      std::unique_ptr<BlotterSettings> m_blotterSettings;
      boost::optional<OrderImbalanceIndicatorWindowSettings>
        m_initialOrderImbalanceIndicatorWindowSettings;
      boost::optional<PortfolioViewerWindowSettings>
        m_initialPortfolioViewerWindowSettings;
  };
}

#endif
