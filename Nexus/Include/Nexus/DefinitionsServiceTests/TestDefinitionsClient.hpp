#ifndef NEXUS_TEST_DEFINITIONS_CLIENT_HPP
#define NEXUS_TEST_DEFINITIONS_CLIENT_HPP
#include <variant>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"

namespace Nexus::Tests {

  /**
   * Implements a DefinitionsClient for testing by pushing all operations
   * performed onto a queue.
   */
  class TestDefinitionsClient {
    public:

      /** Records a call to load_minimum_spire_client_version(). */
      struct LoadMinimumSpireClientVersionOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<std::string> m_result;
      };

      /** Records a call to load_organization_name(). */
      struct LoadOrganizationNameOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<std::string> m_result;
      };

      /** Records a call to load_country_database(). */
      struct LoadCountryDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<CountryDatabase> m_result;
      };

      /** Records a call to load_time_zone_database(). */
      struct LoadTimeZoneDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<boost::local_time::tz_database> m_result;
      };

      /** Records a call to load_currency_database(). */
      struct LoadCurrencyDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<CurrencyDatabase> m_result;
      };

      /** Records a call to load_destination_database(). */
      struct LoadDestinationDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<DestinationDatabase> m_result;
      };

      /** Records a call to load_venue_database(). */
      struct LoadVenueDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<VenueDatabase> m_result;
      };

      /** Records a call to load_exchange_rates(). */
      struct LoadExchangeRatesOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<std::vector<ExchangeRate>> m_result;
      };

      /** Records a call to load_compliance_rule_schemas(). */
      struct LoadComplianceRuleSchemasOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<std::vector<ComplianceRuleSchema>> m_result;
      };

      /** Records a call to load_trading_schedule(). */
      struct LoadTradingScheduleOperation {

        /** Used to return a value to the caller. */
        Beam::Tests::ServiceResult<TradingSchedule> m_result;
      };

      /** A variant covering all possible TestDefinitionsClient operations. */
      using Operation = std::variant<LoadMinimumSpireClientVersionOperation,
        LoadOrganizationNameOperation, LoadCountryDatabaseOperation,
        LoadTimeZoneDatabaseOperation, LoadCurrencyDatabaseOperation,
        LoadDestinationDatabaseOperation, LoadVenueDatabaseOperation,
        LoadExchangeRatesOperation, LoadComplianceRuleSchemasOperation,
        LoadTradingScheduleOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestDefinitionsClient.
       * @param operations The queue to push all operations on.
       */
      explicit TestDefinitionsClient(Beam::ScopedQueueWriter<
        std::shared_ptr<Operation>> operations) noexcept;

      ~TestDefinitionsClient();

      std::string load_minimum_spire_client_version();
      std::string load_organization_name();
      CountryDatabase load_country_database();
      boost::local_time::tz_database load_time_zone_database();
      CurrencyDatabase load_currency_database();
      DestinationDatabase load_destination_database();
      VenueDatabase load_venue_database();
      std::vector<ExchangeRate> load_exchange_rates();
      std::vector<ComplianceRuleSchema> load_compliance_rule_schemas();
      TradingSchedule load_trading_schedule();
      void close();

    private:
      Beam::Tests::TestServiceClientOperationQueue<Operation> m_operations;

      TestDefinitionsClient(const TestDefinitionsClient&) = delete;
      TestDefinitionsClient& operator =(const TestDefinitionsClient&) = delete;
  };

  inline TestDefinitionsClient::TestDefinitionsClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestDefinitionsClient::~TestDefinitionsClient() {
    close();
  }

  inline std::string
      TestDefinitionsClient::load_minimum_spire_client_version() {
    return m_operations.append_result<
      LoadMinimumSpireClientVersionOperation, std::string>();
  }

  inline std::string TestDefinitionsClient::load_organization_name() {
    return m_operations.append_result<
      LoadOrganizationNameOperation, std::string>();
  }

  inline CountryDatabase TestDefinitionsClient::load_country_database() {
    return m_operations.append_result<
      LoadCountryDatabaseOperation, CountryDatabase>();
  }

  inline boost::local_time::tz_database
      TestDefinitionsClient::load_time_zone_database() {
    return m_operations.append_result<
      LoadTimeZoneDatabaseOperation, boost::local_time::tz_database>();
  }

  inline CurrencyDatabase TestDefinitionsClient::load_currency_database() {
    return m_operations.append_result<
      LoadCurrencyDatabaseOperation, CurrencyDatabase>();
  }

  inline DestinationDatabase
      TestDefinitionsClient::load_destination_database() {
    return m_operations.append_result<
      LoadDestinationDatabaseOperation, DestinationDatabase>();
  }

  inline VenueDatabase TestDefinitionsClient::load_venue_database() {
    return m_operations.append_result<LoadVenueDatabaseOperation, VenueDatabase>();
  }

  inline std::vector<ExchangeRate>
      TestDefinitionsClient::load_exchange_rates() {
    return m_operations.append_result<
      LoadExchangeRatesOperation, std::vector<ExchangeRate>>();
  }

  inline std::vector<ComplianceRuleSchema>
      TestDefinitionsClient::load_compliance_rule_schemas() {
    return m_operations.append_result<LoadComplianceRuleSchemasOperation,
      std::vector<ComplianceRuleSchema>>();
  }

  inline TradingSchedule TestDefinitionsClient::load_trading_schedule() {
    return m_operations.append_result<
      LoadTradingScheduleOperation, TradingSchedule>();
  }

  inline void TestDefinitionsClient::close() {
    m_operations.close();
  }
}

#endif
