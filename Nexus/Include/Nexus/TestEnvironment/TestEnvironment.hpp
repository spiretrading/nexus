#ifndef NEXUS_TEST_ENVIRONMENT_HPP
#define NEXUS_TEST_ENVIRONMENT_HPP
#include <utility>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/ConverterQueueWriter.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/RegistryServiceTests/RegistryServiceTestEnvironment.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTestEnvironment.hpp"
#include "Nexus/ComplianceTests/ComplianceTestEnvironment.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTestEnvironment.hpp"
#include "Nexus/TestEnvironment/TestEnvironmentException.hpp"

namespace Nexus {

  /** Maintains the state needed to test a market environment. */
  class TestEnvironment {
    public:

      /**
       * Constructs a TestEnvironment using the current system time and
       * an empty market datastore.
       */
      TestEnvironment();

      /**
       * Constructs a TestEnvironment using an empty market datastore.
       * @param time The time to set the environment to.
       */
      explicit TestEnvironment(boost::posix_time::ptime time);

      /**
       * Constructs a TestEnvironment using historical market data.
       * @param historical_data_store The data store to use for historical
       *        market data.
       */
      explicit TestEnvironment(
        MarketDataService::HistoricalDataStore historical_data_store);

      /**
       * Constructs a TestEnvironment.
       * @param historical_data_store The data store to use for historical
       *        market data.
       * @param time The time to set the environment to.
       */
      TestEnvironment(
        MarketDataService::HistoricalDataStore historical_data_store,
        boost::posix_time::ptime time);

      ~TestEnvironment();

      /** Sets the time. */
      void set(boost::posix_time::ptime time);

      /** Advances the time by a certain amount. */
      void advance(boost::posix_time::time_duration duration);

      /** Publishes an OrderImbalance. */
      void publish(Venue venue, const OrderImbalance& order_imbalance);

      /** Publishes a BboQuote. */
      void publish(const Security& security, const BboQuote& bbo_quote);

      /** Publishes a BookQuote. */
      void publish(const Security& security, const BookQuote& book_quote);

      /** Publishes a TimeAndSale. */
      void publish(const Security& security, const TimeAndSale& time_and_sale);

      /** Updates the price of a BboQuote. */
      void update_bbo_price(const Security& security, Money bid_price,
        Money ask_price, boost::posix_time::ptime timestamp);

      /** Updates the price of a BboQuote. */
      void update_bbo_price(
        const Security& security, Money bid_price, Money ask_price);

      /** Monitors orders submitted to this environment. */
      void monitor_order_submissions(Beam::ScopedQueueWriter<
        std::shared_ptr<const OrderExecutionService::Order>> queue);

      /** Updates a submitted order to OrderStatus NEW. */
      void accept(const OrderExecutionService::Order& order);

      /** Updates a submitted order to OrderStatus REJECTED. */
      void reject(const OrderExecutionService::Order& order);

      /** Updates a submitted order to OrderStatus CANCELED. */
      void cancel(const OrderExecutionService::Order& order);

      /** Fills an order. */
      void fill(const OrderExecutionService::Order& order, Money price,
        Quantity quantity);

      /** Fills an order. */
      void fill(const OrderExecutionService::Order& order, Quantity quantity);

      /** Updates an order. */
      void update(const OrderExecutionService::Order& order,
        const OrderExecutionService::ExecutionReport& report);

      /** Returns the TimeServiceTestEnvironment. */
      Beam::TimeService::Tests::TimeServiceTestEnvironment&
        get_time_environment();

      /** Returns the ServiceLocatorTestEnvironment. */
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
        get_service_locator_environment();

      /** Returns the UidServiceTestEnvironment. */
      Beam::UidService::Tests::UidServiceTestEnvironment& get_uid_environment();

      /** Returns the RegistryServiceTestEnvironment. */
      Beam::RegistryService::Tests::RegistryServiceTestEnvironment&
        get_registry_environment();

      /** Returns the DefinitionsServiceTestEnvironment. */
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
        get_definitions_environment();

      /** Returns the AdministrationServiceTestEnvironment. */
      Tests::AdministrationServiceTestEnvironment&
        get_administration_environment();

      /** Returns the MarketDataServiceTestEnvironment. */
      MarketDataService::Tests::MarketDataServiceTestEnvironment&
        get_market_data_environment();

      /** Returns the ChartingServiceTestEnvironment. */
      ChartingService::Tests::ChartingServiceTestEnvironment&
        get_charting_environment();

      /** Returns the ComplianceTestEnvironment. */
      Compliance::Tests::ComplianceTestEnvironment&
        get_compliance_environment();

      /** Returns the OrderExecutionServiceTestEnvironment. */
      OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
        get_order_execution_environment();

      /** Returns the RiskServiceTestEnvironment. */
      RiskService::Tests::RiskServiceTestEnvironment& get_risk_environment();

      /** Closes the test environment. */
      void close();

    private:
      Beam::TimeService::Tests::TimeServiceTestEnvironment m_time_environment;
      Beam::TimeService::TimeClientBox m_time_client;
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment
        m_service_locator_environment;
      Beam::ServiceLocator::ServiceLocatorClientBox m_service_locator_client;
      Beam::UidService::Tests::UidServiceTestEnvironment m_uid_environment;
      Beam::UidService::UidClientBox m_uid_client;
      Beam::RegistryService::Tests::RegistryServiceTestEnvironment
        m_registry_environment;
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment
        m_definitions_environment;
      Tests::AdministrationServiceTestEnvironment m_administration_environment;
      AdministrationClient m_administration_client;
      boost::optional<
        MarketDataService::Tests::MarketDataServiceTestEnvironment>
          m_market_data_environment;
      boost::optional<MarketDataService::MarketDataClient> m_market_data_client;
      boost::optional<ChartingService::Tests::ChartingServiceTestEnvironment>
        m_charting_environment;
      boost::optional<Compliance::Tests::ComplianceTestEnvironment>
        m_compliance_environment;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment>
          m_order_execution_environment;
      boost::optional<OrderExecutionService::OrderExecutionClient>
        m_order_execution_client;
      boost::optional<RiskService::Tests::RiskServiceTestEnvironment>
        m_risk_environment;
      Beam::IO::OpenState m_open_state;

      TestEnvironment(const TestEnvironment&) = delete;
      TestEnvironment& operator =(const TestEnvironment&) = delete;
      template<typename Index, typename Value>
      void publish_market_data(const Index& index, const Value& value);
  };

  /**
   * Makes a MarketDataClient connected to a TestEnvironment with all available
   * entitlements.
   * @param environment The test environment to connect the MarketDataClient to.
   * @param account_name The name of the account.
   */
  inline MarketDataService::MarketDataClient make_market_data_client(
      TestEnvironment& environment, const std::string& account_name) {
    return MarketDataService::Tests::make_market_data_client(
      environment.get_service_locator_environment(),
      environment.get_administration_environment(),
      environment.get_market_data_environment(), account_name);
  }

  inline TestEnvironment::TestEnvironment()
    : TestEnvironment(MarketDataService::HistoricalDataStore(
        std::in_place_type<MarketDataService::LocalHistoricalDataStore>)) {}

  inline TestEnvironment::TestEnvironment(boost::posix_time::ptime time)
    : TestEnvironment(MarketDataService::HistoricalDataStore(
        std::in_place_type<MarketDataService::LocalHistoricalDataStore>),
        time) {}

  inline TestEnvironment::TestEnvironment(
    MarketDataService::HistoricalDataStore historical_data_store)
    : TestEnvironment(std::move(historical_data_store),
        boost::posix_time::second_clock::universal_time()) {}

  inline TestEnvironment::TestEnvironment(
      MarketDataService::HistoricalDataStore historical_data_store,
      boost::posix_time::ptime time)
    : m_time_environment(time),
      m_time_client(
        std::in_place_type<Beam::TimeService::Tests::TestTimeClient>,
        Beam::Ref(m_time_environment)),
      m_service_locator_client(m_service_locator_environment.MakeClient()),
      m_uid_client(m_uid_environment.MakeClient()),
      m_registry_environment(m_service_locator_client),
      m_definitions_environment(m_service_locator_client),
      m_administration_environment(m_service_locator_client),
      m_administration_client(
        m_administration_environment.make_client(m_service_locator_client)) {
    try {
      auto account = m_service_locator_client.GetAccount();
      m_service_locator_client.Associate(
        account, m_administration_client.load_administrators_root_entry());
      m_service_locator_client.Associate(
        account, m_administration_client.load_services_root_entry());
      m_market_data_environment.emplace(m_service_locator_client,
        m_administration_client, std::move(historical_data_store));
      m_market_data_client.emplace(
        m_market_data_environment->make_registry_client(
          m_service_locator_client));
      m_charting_environment.emplace(
        m_service_locator_client, *m_market_data_client);
      m_compliance_environment.emplace(
        m_service_locator_client, m_administration_client, m_time_client);
      auto definitions_client =
        m_definitions_environment.make_client(m_service_locator_client);
      m_order_execution_environment.emplace(
        definitions_client.load_venue_database(),
        definitions_client.load_destination_database(),
        m_service_locator_client, m_uid_client, m_administration_client);
      m_order_execution_client.emplace(
        m_order_execution_environment->make_client(m_service_locator_client));
      auto transition_timer_factory = [this] {
        return std::make_unique<Beam::Threading::TimerBox>(
          std::make_unique<Beam::TimeService::Tests::TestTimer>(
            boost::posix_time::seconds(1), Beam::Ref(m_time_environment)));
      };
      m_risk_environment.emplace(m_service_locator_client,
        m_administration_client, *m_market_data_client,
        *m_order_execution_client, transition_timer_factory, m_time_client,
        ExchangeRateTable(definitions_client.load_exchange_rates()),
        definitions_client.load_venue_database(),
        definitions_client.load_destination_database());
    } catch(const std::exception&) {
      close();
      BOOST_RETHROW;
    }
  }

  inline TestEnvironment::~TestEnvironment() {
    close();
  }

  inline void TestEnvironment::set(boost::posix_time::ptime time) {
    m_time_environment.SetTime(time);
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::advance(
      boost::posix_time::time_duration duration) {
    m_time_environment.AdvanceTime(duration);
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::publish(
      Venue venue, const OrderImbalance& order_imbalance) {
    publish_market_data(venue, order_imbalance);
  }

  inline void TestEnvironment::publish(
      const Security& security, const BboQuote& bbo_quote) {
    publish_market_data(security, bbo_quote);
  }

  inline void TestEnvironment::publish(
      const Security& security, const BookQuote& book_quote) {
    publish_market_data(security, book_quote);
  }

  inline void TestEnvironment::publish(
      const Security& security, const TimeAndSale& time_and_sale) {
    publish_market_data(security, time_and_sale);
  }

  inline void TestEnvironment::update_bbo_price(
      const Security& security, Money bid_price, Money ask_price,
      boost::posix_time::ptime timestamp) {
    auto quote = BboQuote(Quote(bid_price, 100, Side::BID),
      Quote(ask_price, 100, Side::ASK), timestamp);
    publish(security, quote);
  }

  inline void TestEnvironment::update_bbo_price(
      const Security& security, Money bid_price, Money ask_price) {
    update_bbo_price(
      security, bid_price, ask_price, boost::posix_time::not_a_date_time);
  }

  inline void TestEnvironment::monitor_order_submissions(
      Beam::ScopedQueueWriter<
        std::shared_ptr<const OrderExecutionService::Order>> queue) {
    auto primitive_order_queue = Beam::MakeConverterQueueWriter<
      std::shared_ptr<OrderExecutionService::PrimitiveOrder>>(std::move(queue),
      [] (const auto& order) {
        return std::static_pointer_cast<const OrderExecutionService::Order>(
          order);
      });
    auto& driver = get_order_execution_environment().get_driver().as<
      OrderExecutionService::Tests::MockOrderExecutionDriver>();
    driver.get_publisher().Monitor(std::move(primitive_order_queue));
  }

  inline void TestEnvironment::accept(
      const OrderExecutionService::Order& order) {
    auto primitive_order = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(!primitive_order) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&](auto status, const auto&) {
      if(status != OrderStatus::PENDING_NEW) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order must be PENDING_NEW."));
      }
    });
    OrderExecutionService::Tests::set_order_status(
      *primitive_order, OrderStatus::NEW, m_time_environment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::reject(
      const OrderExecutionService::Order& order) {
    auto primitive_order = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(!primitive_order) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&](auto status, const auto&) {
      if(is_terminal(status)) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    OrderExecutionService::Tests::set_order_status(
      *primitive_order, OrderStatus::REJECTED, m_time_environment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::cancel(
      const OrderExecutionService::Order& order) {
    auto primitive_order = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(!primitive_order) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&](auto status, const auto&) {
      if(is_terminal(status)) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    OrderExecutionService::Tests::cancel(
      *primitive_order, m_time_environment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::fill(const OrderExecutionService::Order& order,
      Money price, Quantity quantity) {
    auto primitive_order = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(!primitive_order) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&](auto status, const auto&) {
      if(is_terminal(status)) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    OrderExecutionService::Tests::fill(
      *primitive_order, price, quantity, m_time_environment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::fill(
      const OrderExecutionService::Order& order, Quantity quantity) {
    fill(order, order.get_info().m_fields.m_price, quantity);
  }

  inline void TestEnvironment::update(const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& report) {
    auto primitive_order = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(!primitive_order) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&](auto status, const auto&) {
      if(is_terminal(status)) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    auto revised_report = report;
    if(revised_report.m_timestamp != boost::posix_time::not_a_date_time) {
      m_time_environment.SetTime(revised_report.m_timestamp);
    } else {
      revised_report.m_timestamp = m_time_environment.GetTime();
    }
    primitive_order->with([&](auto, const auto& reports) {
      auto& last_report = reports.back();
      revised_report.m_id = last_report.m_id;
      revised_report.m_sequence = last_report.m_sequence + 1;
      primitive_order->update(revised_report);
    });
    Beam::Routines::FlushPendingRoutines();
  }

  inline Beam::TimeService::Tests::TimeServiceTestEnvironment&
      TestEnvironment::get_time_environment() {
    return m_time_environment;
  }

  inline Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
      TestEnvironment::get_service_locator_environment() {
    return m_service_locator_environment;
  }

  inline Beam::UidService::Tests::UidServiceTestEnvironment&
      TestEnvironment::get_uid_environment() {
    return m_uid_environment;
  }

  inline Beam::RegistryService::Tests::RegistryServiceTestEnvironment&
      TestEnvironment::get_registry_environment() {
    return m_registry_environment;
  }

  inline DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
      TestEnvironment::get_definitions_environment() {
    return m_definitions_environment;
  }

  inline Tests::AdministrationServiceTestEnvironment&
      TestEnvironment::get_administration_environment() {
    return m_administration_environment;
  }

  inline MarketDataService::Tests::MarketDataServiceTestEnvironment&
      TestEnvironment::get_market_data_environment() {
    return *m_market_data_environment;
  }

  inline ChartingService::Tests::ChartingServiceTestEnvironment&
      TestEnvironment::get_charting_environment() {
    return *m_charting_environment;
  }

  inline Compliance::Tests::ComplianceTestEnvironment&
      TestEnvironment::get_compliance_environment() {
    return *m_compliance_environment;
  }

  inline OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
      TestEnvironment::get_order_execution_environment() {
    return *m_order_execution_environment;
  }

  inline RiskService::Tests::RiskServiceTestEnvironment&
      TestEnvironment::get_risk_environment() {
    return *m_risk_environment;
  }

  inline void TestEnvironment::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_risk_environment->close();
    m_order_execution_client->close();
    m_order_execution_environment->close();
    m_compliance_environment->close();
    m_charting_environment->close();
    m_market_data_client->close();
    m_market_data_environment->close();
    m_administration_client.close();
    m_administration_environment.close();
    m_definitions_environment.close();
    m_registry_environment.Close();
    m_uid_client.Close();
    m_uid_environment.Close();
    m_service_locator_client.Close();
    m_service_locator_environment.Close();
    m_time_client.Close();
    m_time_environment.Close();
    m_open_state.Close();
  }

  template<typename Index, typename Value>
  void TestEnvironment::publish_market_data(
      const Index& index, const Value& value) {
    if(Beam::Queries::GetTimestamp(value) !=
        boost::posix_time::not_a_date_time) {
      m_time_environment.SetTime(Beam::Queries::GetTimestamp(value));
      get_market_data_environment().get_feed_client().publish(
        Beam::Queries::IndexedValue(value, index));
    } else {
      auto revised_value = value;
      Beam::Queries::GetTimestamp(revised_value) = m_time_environment.GetTime();
      get_market_data_environment().get_feed_client().publish(
        Beam::Queries::IndexedValue(revised_value, index));
    }
  }
}

#endif
