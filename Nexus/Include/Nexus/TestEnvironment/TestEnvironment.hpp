#ifndef NEXUS_TEST_ENVIRONMENT_HPP
#define NEXUS_TEST_ENVIRONMENT_HPP
#include <utility>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/ConverterQueueWriter.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
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
      explicit TestEnvironment(HistoricalDataStore historical_data_store);

      /**
       * Constructs a TestEnvironment.
       * @param historical_data_store The data store to use for historical
       *        market data.
       * @param time The time to set the environment to.
       */
      TestEnvironment(HistoricalDataStore historical_data_store,
        boost::posix_time::ptime time);

      ~TestEnvironment();

      /** Sets the time. */
      void set(boost::posix_time::ptime time);

      /** Advances the time by a certain amount. */
      void advance(boost::posix_time::time_duration duration);

      /** Publishes an OrderImbalance. */
      void publish(Venue venue, const OrderImbalance& order_imbalance);

      /** Publishes a BboQuote. */
      void publish(const Ticker& ticker, const BboQuote& bbo_quote);

      /** Publishes a BookQuote. */
      void publish(const Ticker& ticker, const BookQuote& book_quote);

      /** Publishes a TimeAndSale. */
      void publish(const Ticker& ticker, const TimeAndSale& time_and_sale);

      /** Updates the price of a BboQuote. */
      void update_bbo_price(const Ticker& ticker, Money bid_price,
        Money ask_price, boost::posix_time::ptime timestamp);

      /** Updates the price of a BboQuote. */
      void update_bbo_price(
        const Ticker& ticker, Money bid_price, Money ask_price);

      /** Monitors orders submitted to this environment. */
      void monitor_order_submissions(
        Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue);

      /** Updates a submitted order to OrderStatus NEW. */
      void accept(const Order& order);

      /** Updates a submitted order to OrderStatus REJECTED. */
      void reject(const Order& order);

      /** Updates a submitted order to OrderStatus CANCELED. */
      void cancel(const Order& order);

      /** Fills an order. */
      void fill(const Order& order, Money price, Quantity quantity);

      /** Fills an order. */
      void fill(const Order& order, Quantity quantity);

      /** Updates an order. */
      void update(const Order& order, const ExecutionReport& report);

      /** Returns the TimeServiceTestEnvironment. */
      Beam::Tests::TimeServiceTestEnvironment& get_time_environment();

      /** Returns the ServiceLocatorTestEnvironment. */
      Beam::Tests::ServiceLocatorTestEnvironment&
        get_service_locator_environment();

      /** Returns the UidServiceTestEnvironment. */
      Beam::Tests::UidServiceTestEnvironment& get_uid_environment();

      /** Returns the DefinitionsServiceTestEnvironment. */
      Tests::DefinitionsServiceTestEnvironment& get_definitions_environment();

      /** Returns the AdministrationServiceTestEnvironment. */
      Tests::AdministrationServiceTestEnvironment&
        get_administration_environment();

      /** Returns the MarketDataServiceTestEnvironment. */
      Tests::MarketDataServiceTestEnvironment& get_market_data_environment();

      /** Returns the ChartingServiceTestEnvironment. */
      Tests::ChartingServiceTestEnvironment& get_charting_environment();

      /** Returns the ComplianceTestEnvironment. */
      Tests::ComplianceTestEnvironment& get_compliance_environment();

      /** Returns the OrderExecutionServiceTestEnvironment. */
      Tests::OrderExecutionServiceTestEnvironment&
        get_order_execution_environment();

      /** Returns the RiskServiceTestEnvironment. */
      Tests::RiskServiceTestEnvironment& get_risk_environment();

      /** Closes the test environment. */
      void close();

    private:
      Beam::Tests::TimeServiceTestEnvironment m_time_environment;
      Beam::TimeClient m_time_client;
      Beam::Tests::ServiceLocatorTestEnvironment m_service_locator_environment;
      Beam::ServiceLocatorClient m_service_locator_client;
      Beam::Tests::UidServiceTestEnvironment m_uid_environment;
      Beam::UidClient m_uid_client;
      Tests::DefinitionsServiceTestEnvironment m_definitions_environment;
      Tests::AdministrationServiceTestEnvironment m_administration_environment;
      AdministrationClient m_administration_client;
      boost::optional<Tests::MarketDataServiceTestEnvironment>
        m_market_data_environment;
      boost::optional<MarketDataClient> m_market_data_client;
      boost::optional<Tests::ChartingServiceTestEnvironment>
        m_charting_environment;
      boost::optional<Tests::ComplianceTestEnvironment>
        m_compliance_environment;
      boost::optional<Tests::OrderExecutionServiceTestEnvironment>
        m_order_execution_environment;
      boost::optional<OrderExecutionClient> m_order_execution_client;
      boost::optional<Tests::RiskServiceTestEnvironment> m_risk_environment;
      Beam::OpenState m_open_state;

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
  inline MarketDataClient make_market_data_client(
      TestEnvironment& environment, const std::string& account_name) {
    return Tests::make_market_data_client(
      environment.get_service_locator_environment(),
      environment.get_administration_environment(),
      environment.get_market_data_environment(), account_name);
  }

  inline TestEnvironment::TestEnvironment()
    : TestEnvironment(
        HistoricalDataStore(std::in_place_type<LocalHistoricalDataStore>)) {}

  inline TestEnvironment::TestEnvironment(boost::posix_time::ptime time)
    : TestEnvironment(HistoricalDataStore(
        std::in_place_type<LocalHistoricalDataStore>), time) {}

  inline TestEnvironment::TestEnvironment(
    HistoricalDataStore historical_data_store)
    : TestEnvironment(std::move(historical_data_store),
        boost::posix_time::second_clock::universal_time()) {}

  inline TestEnvironment::TestEnvironment(
      HistoricalDataStore historical_data_store, boost::posix_time::ptime time)
    : m_time_environment(time),
      m_time_client(std::in_place_type<Beam::Tests::TestTimeClient>,
        Beam::Ref(m_time_environment)),
      m_service_locator_client(m_service_locator_environment.make_client()),
      m_uid_client(m_uid_environment.make_client()),
      m_definitions_environment(m_service_locator_client),
      m_administration_environment(m_service_locator_client),
      m_administration_client(m_administration_environment.make_client(
        Beam::Ref(m_service_locator_client))) {
    try {
      auto account = m_service_locator_client.get_account();
      m_service_locator_client.associate(
        account, m_administration_client.load_administrators_root_entry());
      m_service_locator_client.associate(
        account, m_administration_client.load_services_root_entry());
      m_market_data_environment.emplace(m_service_locator_client,
        m_administration_client, std::move(historical_data_store));
      m_market_data_client.emplace(
        m_market_data_environment->make_registry_client(
          Beam::Ref(m_service_locator_client)));
      m_charting_environment.emplace(
        m_service_locator_client, *m_market_data_client);
      m_compliance_environment.emplace(
        m_service_locator_client, m_administration_client, m_time_client);
      auto definitions_client = m_definitions_environment.make_client(
        Beam::Ref(m_service_locator_client));
      m_order_execution_environment.emplace(
        definitions_client.load_venue_database(),
        definitions_client.load_destination_database(),
        m_service_locator_client, m_uid_client, m_administration_client);
      m_order_execution_client.emplace(
        m_order_execution_environment->make_client(
          Beam::Ref(m_service_locator_client)));
      auto transition_timer_factory = [this] {
        return std::make_unique<Beam::Timer>(
          std::make_unique<Beam::Tests::TestTimer>(
            boost::posix_time::seconds(1), Beam::Ref(m_time_environment)));
      };
      m_risk_environment.emplace(m_service_locator_client,
        m_administration_client, *m_market_data_client,
        *m_order_execution_client, transition_timer_factory, m_time_client,
        ExchangeRateTable(definitions_client.load_exchange_rates()),
        definitions_client.load_destination_database());
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  inline TestEnvironment::~TestEnvironment() {
    close();
  }

  inline void TestEnvironment::set(boost::posix_time::ptime time) {
    m_time_environment.set(time);
    Beam::flush_pending_routines();
  }

  inline void TestEnvironment::advance(
      boost::posix_time::time_duration duration) {
    m_time_environment.advance(duration);
    Beam::flush_pending_routines();
  }

  inline void TestEnvironment::publish(
      Venue venue, const OrderImbalance& order_imbalance) {
    publish_market_data(venue, order_imbalance);
  }

  inline void TestEnvironment::publish(
      const Ticker& ticker, const BboQuote& bbo_quote) {
    publish_market_data(ticker, bbo_quote);
  }

  inline void TestEnvironment::publish(
      const Ticker& ticker, const BookQuote& book_quote) {
    publish_market_data(ticker, book_quote);
  }

  inline void TestEnvironment::publish(
      const Ticker& ticker, const TimeAndSale& time_and_sale) {
    publish_market_data(ticker, time_and_sale);
  }

  inline void TestEnvironment::update_bbo_price(
      const Ticker& ticker, Money bid_price, Money ask_price,
      boost::posix_time::ptime timestamp) {
    auto quote =
      BboQuote(make_bid(bid_price, 100), make_ask(ask_price, 100), timestamp);
    publish(ticker, quote);
  }

  inline void TestEnvironment::update_bbo_price(
      const Ticker& ticker, Money bid_price, Money ask_price) {
    update_bbo_price(
      ticker, bid_price, ask_price, boost::posix_time::not_a_date_time);
  }

  inline void TestEnvironment::monitor_order_submissions(
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    auto primitive_order_queue =
      Beam::convert<std::shared_ptr<PrimitiveOrder>>(std::move(queue),
        [] (const auto& order) {
          return std::static_pointer_cast<Order>(order);
        });
    auto& driver = get_order_execution_environment().get_driver().as<
      Tests::MockOrderExecutionDriver>();
    driver.get_publisher().monitor(std::move(primitive_order_queue));
  }

  inline void TestEnvironment::accept(const Order& order) {
    auto primitive_order =
      const_cast<PrimitiveOrder*>(dynamic_cast<const PrimitiveOrder*>(&order));
    if(!primitive_order) {
      boost::throw_with_location(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&] (auto status, const auto&) {
      if(status != OrderStatus::PENDING_NEW) {
        boost::throw_with_location(
          TestEnvironmentException("Order must be PENDING_NEW."));
      }
    });
    Tests::set_order_status(
      *primitive_order, OrderStatus::NEW, m_time_environment.get_time());
    Beam::flush_pending_routines();
  }

  inline void TestEnvironment::reject(const Order& order) {
    auto primitive_order =
      const_cast<PrimitiveOrder*>(dynamic_cast<const PrimitiveOrder*>(&order));
    if(!primitive_order) {
      boost::throw_with_location(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&] (auto status, const auto&) {
      if(is_terminal(status)) {
        boost::throw_with_location(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    Tests::set_order_status(
      *primitive_order, OrderStatus::REJECTED, m_time_environment.get_time());
    Beam::flush_pending_routines();
  }

  inline void TestEnvironment::cancel(const Order& order) {
    auto primitive_order =
      const_cast<PrimitiveOrder*>(dynamic_cast<const PrimitiveOrder*>(&order));
    if(!primitive_order) {
      boost::throw_with_location(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&] (auto status, const auto&) {
      if(is_terminal(status)) {
        boost::throw_with_location(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    Tests::cancel(*primitive_order, m_time_environment.get_time());
    Beam::flush_pending_routines();
  }

  inline void TestEnvironment::fill(
      const Order& order, Money price, Quantity quantity) {
    auto primitive_order =
      const_cast<PrimitiveOrder*>(dynamic_cast<const PrimitiveOrder*>(&order));
    if(!primitive_order) {
      boost::throw_with_location(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&](auto status, const auto&) {
      if(is_terminal(status)) {
        boost::throw_with_location(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    Tests::fill(
      *primitive_order, price, quantity, m_time_environment.get_time());
    Beam::flush_pending_routines();
  }

  inline void TestEnvironment::fill(const Order& order, Quantity quantity) {
    fill(order, order.get_info().m_fields.m_price, quantity);
  }

  inline void TestEnvironment::update(
      const Order& order, const ExecutionReport& report) {
    auto primitive_order =
      const_cast<PrimitiveOrder*>(dynamic_cast<const PrimitiveOrder*>(&order));
    if(!primitive_order) {
      boost::throw_with_location(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitive_order->with([&] (auto status, const auto&) {
      if(is_terminal(status)) {
        boost::throw_with_location(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    auto revised_report = report;
    if(revised_report.m_timestamp != boost::posix_time::not_a_date_time) {
      m_time_environment.set(revised_report.m_timestamp);
    } else {
      revised_report.m_timestamp = m_time_environment.get_time();
    }
    primitive_order->with([&] (auto, const auto& reports) {
      auto& last_report = reports.back();
      revised_report.m_id = last_report.m_id;
      revised_report.m_sequence = last_report.m_sequence + 1;
      primitive_order->update(revised_report);
    });
    Beam::flush_pending_routines();
  }

  inline Beam::Tests::TimeServiceTestEnvironment&
      TestEnvironment::get_time_environment() {
    return m_time_environment;
  }

  inline Beam::Tests::ServiceLocatorTestEnvironment&
      TestEnvironment::get_service_locator_environment() {
    return m_service_locator_environment;
  }

  inline Beam::Tests::UidServiceTestEnvironment&
      TestEnvironment::get_uid_environment() {
    return m_uid_environment;
  }

  inline Tests::DefinitionsServiceTestEnvironment&
      TestEnvironment::get_definitions_environment() {
    return m_definitions_environment;
  }

  inline Tests::AdministrationServiceTestEnvironment&
      TestEnvironment::get_administration_environment() {
    return m_administration_environment;
  }

  inline Tests::MarketDataServiceTestEnvironment&
      TestEnvironment::get_market_data_environment() {
    return *m_market_data_environment;
  }

  inline Tests::ChartingServiceTestEnvironment&
      TestEnvironment::get_charting_environment() {
    return *m_charting_environment;
  }

  inline Tests::ComplianceTestEnvironment&
      TestEnvironment::get_compliance_environment() {
    return *m_compliance_environment;
  }

  inline Tests::OrderExecutionServiceTestEnvironment&
      TestEnvironment::get_order_execution_environment() {
    return *m_order_execution_environment;
  }

  inline Tests::RiskServiceTestEnvironment&
      TestEnvironment::get_risk_environment() {
    return *m_risk_environment;
  }

  inline void TestEnvironment::close() {
    if(m_open_state.set_closing()) {
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
    m_uid_client.close();
    m_uid_environment.close();
    m_service_locator_client.close();
    m_service_locator_environment.close();
    m_time_client.close();
    m_time_environment.close();
    m_open_state.close();
  }

  template<typename Index, typename Value>
  void TestEnvironment::publish_market_data(
      const Index& index, const Value& value) {
    if(Beam::get_timestamp(value) != boost::posix_time::not_a_date_time) {
      m_time_environment.set(Beam::get_timestamp(value));
      get_market_data_environment().get_feed_client().publish(
        Beam::IndexedValue(value, index));
    } else {
      auto revised_value = value;
      Beam::get_timestamp(revised_value) = m_time_environment.get_time();
      get_market_data_environment().get_feed_client().publish(
        Beam::IndexedValue(revised_value, index));
    }
  }
}

#endif
