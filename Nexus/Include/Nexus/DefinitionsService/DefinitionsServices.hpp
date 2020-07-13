#ifndef NEXUS_DEFINITIONS_SERVICES_HPP
#define NEXUS_DEFINITIONS_SERVICES_HPP
#include <vector>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus::DefinitionsService {
  BEAM_DEFINE_SERVICES(DefinitionsServices,

    /**
     * Loads the minimum version of the Spire client needed to login.
     * @param dummy Unused.
     * @return The minimum version of the Spire client needed to login.
     */
    (LoadMinimumSpireClientVersionService,
      "Nexus.DefinitionsServices.LoadMinimumSpireClientVersionService",
      std::string, int, dummy),

    /**
     * Loads the name of the organization.
     * @param dummy Unused.
     * @return The name of the organization using this instance.
     */
    (LoadOrganizationNameService,
      "Nexus.DefinitionsServices.LoadOrganizationNameService", std::string,
      int, dummy),

    /**
     * Loads the CountryDatabase.
     * @param dummy Unused.
     * @return The CountryDatabase.
     */
    (LoadCountryDatabaseService,
      "Nexus.DefinitionsServices.LoadCountryDatabaseService", CountryDatabase,
      int, dummy),

    /**
     * Loads the time zone database.
     * @param dummy Unused.
     * @return The string representation of available time zones.
     */
    (LoadTimeZoneDatabaseService,
      "Nexus.DefinitionsServices.LoadTimeZoneDatabaseService", std::string,
      int, dummy),

    /**
     * Loads the CurrencyDatabase.
     * @param dummy Unused.
     * @return The CurrencyDatabase.
     */
    (LoadCurrencyDatabaseService,
      "Nexus.DefinitionsServices.LoadCurrencyDatabaseService", CurrencyDatabase,
      int, dummy),

    /**
     * Loads the DestinationDatabase.
     * @param dummy Unused.
     * @return The DestinationDatabase.
     */
    (LoadDestinationDatabaseService,
      "Nexus.DefinitionsServices.LoadDestinationDatabaseService",
      DestinationDatabase, int, dummy),

    /**
     * Loads the MarketDatabase.
     * @param dummy Unused.
     * @return The MarketDatabase.
     */
    (LoadMarketDatabaseService,
      "Nexus.DefinitionsServices.LoadMarketDatabaseService", MarketDatabase,
      int, dummy),

    /**
     * Loads the list of ExchangeRates.
     * @param dummy Unused.
     * @return The list of ExchangeRates.
     */
    (LoadExchangeRatesService,
      "Nexus.DefinitionsServices.LoadExchangeRatesService",
      std::vector<ExchangeRate>, int, dummy),

    /**
     * Loads the list of ComplianceRuleSchemas.
     * @param dummy Unused.
     * @return The list of ComplianceRuleSchemas.
     */
    (LoadComplianceRuleSchemasService,
      "Nexus.DefinitionsServices.LoadComplianceRuleSchemasService",
      std::vector<Compliance::ComplianceRuleSchema>, int, dummy));
}

#endif
