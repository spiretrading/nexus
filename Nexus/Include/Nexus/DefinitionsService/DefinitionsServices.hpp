#ifndef NEXUS_DEFINITIONS_SERVICES_HPP
#define NEXUS_DEFINITIONS_SERVICES_HPP
#include <string>
#include <vector>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Standard name for the definitions service. */
  inline const auto DEFINITIONS_SERVICE_NAME =
    std::string("definitions_service");

  BEAM_DEFINE_SERVICES(definitions_services,

    /**
     * Loads the minimum version of the Spire client needed to login.
     * @return The minimum version of the Spire client needed to login.
     */
    (LoadMinimumSpireClientVersionService,
      "Nexus.DefinitionsServices.LoadMinimumSpireClientVersionService",
      std::string),

    /**
     * Loads the name of the organization.
     * @return The name of the organization using this instance.
     */
    (LoadOrganizationNameService,
      "Nexus.DefinitionsServices.LoadOrganizationNameService", std::string),

    /**
     * Loads the CountryDatabase.
     * @return The CountryDatabase.
     */
    (LoadCountryDatabaseService,
      "Nexus.DefinitionsServices.LoadCountryDatabaseService", CountryDatabase),

    /**
     * Loads the time zone database.
     * @return The string representation of available time zones.
     */
    (LoadTimeZoneDatabaseService,
      "Nexus.DefinitionsServices.LoadTimeZoneDatabaseService", std::string),

    /**
     * Loads the CurrencyDatabase.
     * @return The CurrencyDatabase.
     */
    (LoadCurrencyDatabaseService,
      "Nexus.DefinitionsServices.LoadCurrencyDatabaseService",
      CurrencyDatabase),

    /**
     * Loads the DestinationDatabase.
     * @return The DestinationDatabase.
     */
    (LoadDestinationDatabaseService,
      "Nexus.DefinitionsServices.LoadDestinationDatabaseService",
      DestinationDatabase),

    /**
     * Loads the VenueDatabase.
     * @return The VenueDatabase.
     */
    (LoadVenueDatabaseService,
      "Nexus.DefinitionsServices.LoadVenueDatabaseService", VenueDatabase),

    /**
     * Loads the list of ExchangeRates.
     * @return The list of ExchangeRates.
     */
    (LoadExchangeRatesService,
      "Nexus.DefinitionsServices.LoadExchangeRatesService",
      std::vector<ExchangeRate>),

    /**
     * Loads the list of ComplianceRuleSchemas.
     * @return The list of ComplianceRuleSchemas.
     */
    (LoadComplianceRuleSchemasService,
      "Nexus.DefinitionsServices.LoadComplianceRuleSchemasService",
      std::vector<ComplianceRuleSchema>),

    /**
     * Loads a single day's trading schedule.
     * @return The TradingSchedule.
     */
    (LoadTradingScheduleService,
      "Nexus.DefinitionsServices.LoadTradingScheduleService", TradingSchedule));
}

#endif
