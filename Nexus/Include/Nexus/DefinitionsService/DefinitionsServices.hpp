#ifndef NEXUS_DEFINITIONSSERVICES_HPP
#define NEXUS_DEFINITIONSSERVICES_HPP
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

namespace Nexus {
namespace DefinitionsService {
  BEAM_DEFINE_SERVICES(DefinitionsServices,

    /*! \interface Nexus::DefinitionsServices::LoadMinimumSpireClientVersionService
        \brief Loads the minimum version of the Spire client needed to login.
        \param dummy <code>int</code> Unused
        \return <code>std::string</code> The minimum version of the Spire client
                needed to login.
    */
    //! \cond
    (LoadMinimumSpireClientVersionService,
      "Nexus.DefinitionsServices.LoadMinimumSpireClientVersionService",
      std::string, int, dummy),
    //! \endcond

    /*! \interface Nexus::DefinitionsServices::LoadCountryDatabaseService
        \brief Loads the CountryDatabase.
        \param dummy <code>int</code> Unused
        \return <code>CountryDatabase</code> The CountryDatabase.
    */
    //! \cond
    (LoadCountryDatabaseService,
      "Nexus.DefinitionsServices.LoadCountryDatabaseService", CountryDatabase,
      int, dummy),
    //! \endcond

    /*! \interface Nexus::DefinitionsServices::LoadTimeZoneDatabaseService
        \brief Loads the time zone database.
        \param dummy <code>int</code> Unused
        \return <code>std::string</code> The string representation of available
                time zones.
    */
    //! \cond
    (LoadTimeZoneDatabaseService,
      "Nexus.DefinitionsServices.LoadTimeZoneDatabaseService", std::string,
      int, dummy),
    //! \endcond

    /*! \interface Nexus::DefinitionsServices::LoadCurrencyDatabaseService
        \brief Loads the CurrencyDatabase.
        \param dummy <code>int</code> Unused
        \return <code>CurrencyDatabase</code> The CurrencyDatabase.
    */
    //! \cond
    (LoadCurrencyDatabaseService,
      "Nexus.DefinitionsServices.LoadCurrencyDatabaseService", CurrencyDatabase,
      int, dummy),
    //! \endcond

    /*! \interface Nexus::DefinitionsServices::LoadDestinationDatabase
        \brief Loads the DestinationDatabase.
        \param dummy <code>int</code> Unused
        \return <code>DestinationDatabase</code> The DestinationDatabase.
    */
    //! \cond
    (LoadDestinationDatabaseService,
      "Nexus.DefinitionsServices.LoadDestinationDatabaseService",
      DestinationDatabase, int, dummy),
    //! \endcond

    /*! \interface Nexus::DefinitionsServices::LoadMarketDatabaseService
        \brief Loads the MarketDatabase.
        \param dummy <code>int</code> Unused
        \return <code>MarketDatabase</code> The MarketDatabase.
    */
    //! \cond
    (LoadMarketDatabaseService,
      "Nexus.DefinitionsServices.LoadMarketDatabaseService", MarketDatabase,
      int, dummy),

    /*! \interface Nexus::DefinitionsServices::LoadExchangeRatesService
        \brief Loads the list of ExchangeRates.
        \param dummy <code>int</code> Unused
        \return <code>std::vector\<ExchangeRate\></code> The list of
                ExchangeRates.
    */
    //! \cond
    (LoadExchangeRatesService,
      "Nexus.DefinitionsServices.LoadExchangeRatesService",
      std::vector<ExchangeRate>, int, dummy),
    //! \endcond

    /*! \interface Nexus::DefinitionsServices::LoadComplianceRuleSchemasService
        \brief Loads the list of ComplianceRuleSchemas.
        \param dummy <code>int</code> Unused
        \return <code>std::vector\<ComplianceRuleSchema\></code> The list of
                ComplianceRuleSchemas.
    */
    //! \cond
    (LoadComplianceRuleSchemasService,
      "Nexus.DefinitionsServices.LoadComplianceRuleSchemasService",
      std::vector<Compliance::ComplianceRuleSchema>, int, dummy));
    //! \endcond
}
}

#endif
