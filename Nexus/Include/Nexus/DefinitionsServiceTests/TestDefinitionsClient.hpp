#ifndef NEXUS_TEST_DEFINITIONS_CLIENT_HPP
#define NEXUS_TEST_DEFINITIONS_CLIENT_HPP
#include <variant>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <boost/throw_exception.hpp>
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
        Beam::Services::Tests::ServiceResult<std::string> m_result;
      };

      /** Records a call to load_organization_name(). */
      struct LoadOrganizationNameOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<std::string> m_result;
      };

      /** Records a call to load_country_database(). */
      struct LoadCountryDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<CountryDatabase> m_result;
      };

      /** Records a call to load_time_zone_database(). */
      struct LoadTimeZoneDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<boost::local_time::tz_database>
          m_result;
      };

      /** Records a call to load_currency_database(). */
      struct LoadCurrencyDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<CurrencyDatabase> m_result;
      };

      /** Records a call to load_destination_database(). */
      struct LoadDestinationDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<DestinationDatabase> m_result;
      };

      /** Records a call to load_venue_database(). */
      struct LoadVenueDatabaseOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<VenueDatabase> m_result;
      };

      /** Records a call to load_exchange_rates(). */
      struct LoadExchangeRatesOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<std::vector<ExchangeRate>>
          m_result;
      };

      /** Records a call to load_compliance_rule_schemas(). */
      struct LoadComplianceRuleSchemasOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<std::vector<ComplianceRuleSchema>>
          m_result;
      };

      /** Records a call to load_trading_schedule(). */
      struct LoadTradingScheduleOperation {

        /** Used to return a value to the caller. */
        Beam::Services::Tests::ServiceResult<TradingSchedule> m_result;
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
      Beam::ScopedQueueWriter<std::shared_ptr<Operation>> m_operations;
      Beam::SynchronizedUnorderedSet<Beam::Services::Tests::BaseServiceResult*>
        m_pending_results;
      Beam::IO::OpenState m_open_state;

      TestDefinitionsClient(const TestDefinitionsClient&) = delete;
      TestDefinitionsClient& operator =(const TestDefinitionsClient&) = delete;
      template<typename T, typename R, typename... Args>
      R append_result(Args&&... args);
  };

  inline TestDefinitionsClient::TestDefinitionsClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestDefinitionsClient::~TestDefinitionsClient() {
    close();
  }

  inline std::string
      TestDefinitionsClient::load_minimum_spire_client_version() {
    return append_result<LoadMinimumSpireClientVersionOperation, std::string>();
  }

  inline std::string TestDefinitionsClient::load_organization_name() {
    return append_result<LoadOrganizationNameOperation, std::string>();
  }

  inline CountryDatabase TestDefinitionsClient::load_country_database() {
    return append_result<LoadCountryDatabaseOperation, CountryDatabase>();
  }

  inline boost::local_time::tz_database
      TestDefinitionsClient::load_time_zone_database() {
    return append_result<
      LoadTimeZoneDatabaseOperation, boost::local_time::tz_database>();
  }

  inline CurrencyDatabase TestDefinitionsClient::load_currency_database() {
    return append_result<LoadCurrencyDatabaseOperation, CurrencyDatabase>();
  }

  inline DestinationDatabase
      TestDefinitionsClient::load_destination_database() {
    return append_result<
      LoadDestinationDatabaseOperation, DestinationDatabase>();
  }

  inline VenueDatabase TestDefinitionsClient::load_venue_database() {
    return append_result<LoadVenueDatabaseOperation, VenueDatabase>();
  }

  inline std::vector<ExchangeRate>
      TestDefinitionsClient::load_exchange_rates() {
    return append_result<
      LoadExchangeRatesOperation, std::vector<ExchangeRate>>();
  }

  inline std::vector<ComplianceRuleSchema>
      TestDefinitionsClient::load_compliance_rule_schemas() {
    return append_result<
      LoadComplianceRuleSchemasOperation, std::vector<ComplianceRuleSchema>>();
  }

  inline TradingSchedule TestDefinitionsClient::load_trading_schedule() {
    return append_result<LoadTradingScheduleOperation, TradingSchedule>();
  }

  inline void TestDefinitionsClient::close() {
    if(m_open_state.SetClosing()) {
      m_pending_results.With([&] (auto& results) {
        for(auto& result : results) {
          result->set(std::make_exception_ptr(Beam::IO::EndOfFileException()));
        }
      });
      m_pending_results.Clear();
    }
    m_open_state.Close();
  }

  template<typename T, typename R, typename... Args>
  R TestDefinitionsClient::append_result(Args&&... args) {
    auto async = Beam::Routines::Async<R>();
    auto operation = std::make_shared<Operation>(
      std::in_place_type<T>, std::forward<Args>(args)..., async.GetEval());
    m_pending_results.Insert(&std::get<T>(*operation).m_result);
    if(!m_open_state.IsOpen()) {
      m_pending_results.Erase(&std::get<T>(*operation).m_result);
      BOOST_THROW_EXCEPTION(Beam::IO::EndOfFileException());
    }
    m_operations.Push(operation);
    auto result = std::move(async.Get());
    m_pending_results.Erase(&std::get<T>(*operation).m_result);
    return result;
  }
}

#endif
