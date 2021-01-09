#ifndef NEXUS_TEST_ENVIRONMENT_HPP
#define NEXUS_TEST_ENVIRONMENT_HPP
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
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTestEnvironment.hpp"
#include "Nexus/ComplianceTests/ComplianceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTestEnvironment.hpp"
#include "Nexus/ServiceClients/TestEnvironmentException.hpp"

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
      TestEnvironment(boost::posix_time::ptime time);

      /**
       * Constructs a TestEnvironment using historical market data.
       * @param historicalDataStore The data store to use for historical market
       *        data.
       */
      TestEnvironment(
        MarketDataService::HistoricalDataStoreBox historicalDataStore);

      /**
       * Constructs a TestEnvironment.
       * @param historicalDataStore The data store to use for historical market
       *        data.
       * @param time The time to set the environment to.
       */
      TestEnvironment(
        MarketDataService::HistoricalDataStoreBox historicalDataStore,
        boost::posix_time::ptime time);

      ~TestEnvironment();

      /**
       * Sets the time.
       * @param time The time to set the environment to.
       */
      void SetTime(boost::posix_time::ptime time);

      /**
       * Advances the time by a certain amount.
       * @param duration The amount of time to advance the environment by.
       */
      void AdvanceTime(boost::posix_time::time_duration duration);

      /**
       * Publishes an OrderImbalance.
       * @param market The market to publish to.
       * @param orderImbalance The OrderImbalance to publish.
       */
      void Publish(MarketCode market, const OrderImbalance& orderImbalance);

      /**
       * Publishes a BboQuote.
       * @param security The Security to publish to.
       * @param bboQuote The BboQuote to publish.
       */
      void Publish(const Security& security, const BboQuote& bboQuote);

      /**
       * Publishes a BookQuote.
       * @param security The Security to publish to.
       * @param bookQuote The BookQuote to publish.
       */
      void Publish(const Security& security, const BookQuote& bookQuote);

      /**
       * Publishes a MarketQuote.
       * @param security The Security to publish to.
       * @param marketQuote The MarketQuote to publish.
       */
      void Publish(const Security& security, const MarketQuote& marketQuote);

      /**
       * Publishes a TimeAndSale.
       * @param security The Security to publish to.
       * @param timeAndSale The TimeAndSale to publish.
       */
      void Publish(const Security& security, const TimeAndSale& timeAndSale);

      /**
       * Updates the price of a BboQuote.
       * @param security The Security to update.
       * @param bidPrice The updated bid price.
       * @param askPrice The updated ask price.
       * @param timestamp The timestamp.
       */
      void UpdateBboPrice(const Security& security, Money bidPrice,
        Money askPrice, boost::posix_time::ptime timestamp);

      /**
       * Updates the price of a BboQuote.
       * @param security The Security to update.
       * @param bidPrice The updated bid price.
       * @param askPrice The updated ask price.
       */
      void UpdateBboPrice(const Security& security, Money bidPrice,
        Money askPrice);

      /**
       * Monitors Orders submitted to this environment.
       * @param queue The Queue to publish submitted Orders to.
       */
      void MonitorOrderSubmissions(
        Beam::ScopedQueueWriter<const OrderExecutionService::Order*> queue);

      /**
       * Updates a submitted order to OrderStatus NEW.
       * @param order The Order to accept.
       */
      void Accept(const OrderExecutionService::Order& order);

      /**
       * Updates a submitted order to OrderStatus REJECTED.
       * @param order The Order to reject.
       */
      void Reject(const OrderExecutionService::Order& order);

      /**
       * Updates a submitted order to OrderStatus CANCELED.
       * @param order The Order to reject.
       */
      void Cancel(const OrderExecutionService::Order& order);

      /**
       * Fills an Order.
       * @param order The Order to fill.
       * @param price The price of the fill.
       * @param quantity The Quantity to fill the <i>order</i> for.
       */
      void Fill(const OrderExecutionService::Order& order, Money price,
        Quantity quantity);

      /**
       * Fills an Order.
       * @param order The Order to fill.
       * @param quantity The Quantity to fill the <i>order</i> for.
       */
      void Fill(const OrderExecutionService::Order& order, Quantity quantity);

      /**
       * Updates an Order.
       * @param order The Order to update.
       * @param executionReport The ExecutionReport to update the <i>order</i>
       *        with.
       */
      void Update(const OrderExecutionService::Order& order,
        const OrderExecutionService::ExecutionReport& executionReport);

      /** Returns the TimeServiceTestEnvironment. */
      Beam::TimeService::Tests::TimeServiceTestEnvironment&
        GetTimeEnvironment();

      /** Returns the ServiceLocatorTestEnvironment. */
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
        GetServiceLocatorEnvironment();

      /** Returns the UidServiceTestEnvironment. */
      Beam::UidService::Tests::UidServiceTestEnvironment& GetUidEnvironment();

      /** Returns the RegistryServiceTestEnvironment. */
      Beam::RegistryService::Tests::RegistryServiceTestEnvironment&
        GetRegistryEnvironment();

      /** Returns the DefinitionsServiceTestEnvironment. */
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
        GetDefinitionsEnvironment();

      /** Returns the AdministrationServiceTestEnvironment. */
      AdministrationService::Tests::AdministrationServiceTestEnvironment&
        GetAdministrationEnvironment();

      /** Returns the MarketDataServiceTestEnvironment. */
      MarketDataService::Tests::MarketDataServiceTestEnvironment&
        GetMarketDataEnvironment();

      /** Returns the ChartingServiceTestEnvironment. */
      ChartingService::Tests::ChartingServiceTestEnvironment&
        GetChartingEnvironment();

      /** Returns the ComplianceTestEnvironment. */
      Compliance::Tests::ComplianceTestEnvironment&
        GetComplianceEnvironment();

      /** Returns the OrderExecutionServiceTestEnvironment. */
      OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
        GetOrderExecutionEnvironment();

      /** Returns the RiskServiceTestEnvironment. */
      RiskService::Tests::RiskServiceTestEnvironment& GetRiskEnvironment();

      void Close();

    private:
      Beam::TimeService::Tests::TimeServiceTestEnvironment m_timeEnvironment;
      Beam::TimeService::TimeClientBox m_timeClient;
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment
        m_serviceLocatorEnvironment;
      Beam::ServiceLocator::ServiceLocatorClientBox m_serviceLocatorClient;
      Beam::UidService::Tests::UidServiceTestEnvironment m_uidEnvironment;
      Beam::UidService::UidClientBox m_uidClient;
      Beam::RegistryService::Tests::RegistryServiceTestEnvironment
        m_registryEnvironment;
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment
        m_definitionsEnvironment;
      AdministrationService::Tests::AdministrationServiceTestEnvironment
        m_administrationEnvironment;
      AdministrationService::AdministrationClientBox m_administrationClient;
      boost::optional<
        MarketDataService::Tests::MarketDataServiceTestEnvironment>
        m_marketDataEnvironment;
      boost::optional<MarketDataService::MarketDataClientBox>
        m_marketDataClient;
      boost::optional<ChartingService::Tests::ChartingServiceTestEnvironment>
        m_chartingEnvironment;
      boost::optional<Compliance::Tests::ComplianceTestEnvironment>
        m_complianceEnvironment;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment>
        m_orderExecutionEnvironment;
      boost::optional<OrderExecutionService::OrderExecutionClientBox>
        m_orderExecutionClient;
      boost::optional<RiskService::Tests::RiskServiceTestEnvironment>
        m_riskEnvironment;
      Beam::IO::OpenState m_openState;

      TestEnvironment(const TestEnvironment&) = delete;
      TestEnvironment& operator =(const TestEnvironment&) = delete;
      template<typename Index, typename Value>
      void PublishMarketData(const Index& index, const Value& value);
  };

  inline TestEnvironment::TestEnvironment()
    : TestEnvironment(MarketDataService::HistoricalDataStoreBox(
        std::in_place_type<MarketDataService::LocalHistoricalDataStore>)) {}

  inline TestEnvironment::TestEnvironment(boost::posix_time::ptime time)
    : TestEnvironment(MarketDataService::HistoricalDataStoreBox(
        std::in_place_type<MarketDataService::LocalHistoricalDataStore>),
        time) {}

  inline TestEnvironment::TestEnvironment(
    MarketDataService::HistoricalDataStoreBox historicalDataStore)
    : TestEnvironment(std::move(historicalDataStore),
        boost::posix_time::second_clock::universal_time()) {}

  inline TestEnvironment::TestEnvironment(
      MarketDataService::HistoricalDataStoreBox historicalDataStore,
      boost::posix_time::ptime time)
      : m_timeEnvironment(time),
        m_timeClient(std::in_place_type<
          Beam::TimeService::Tests::TestTimeClient>,
          Beam::Ref(m_timeEnvironment)),
        m_serviceLocatorClient(m_serviceLocatorEnvironment.MakeClient()),
        m_uidClient(m_uidEnvironment.MakeClient()),
        m_registryEnvironment(m_serviceLocatorClient),
        m_definitionsEnvironment(m_serviceLocatorClient),
        m_administrationEnvironment(m_serviceLocatorClient),
        m_administrationClient(m_administrationEnvironment.MakeClient(
          m_serviceLocatorClient)) {
    try {
      auto rootAccount = m_serviceLocatorClient.GetAccount();
      m_serviceLocatorClient.Associate(rootAccount,
        m_administrationClient.LoadAdministratorsRootEntry());
      m_serviceLocatorClient.Associate(rootAccount,
        m_administrationClient.LoadServicesRootEntry());
      m_marketDataEnvironment.emplace(m_serviceLocatorClient,
        m_administrationClient, std::move(historicalDataStore));
      m_marketDataClient.emplace(m_marketDataEnvironment->MakeRegistryClient(
        m_serviceLocatorClient));
      m_chartingEnvironment.emplace(m_serviceLocatorClient,
        *m_marketDataClient);
      m_complianceEnvironment.emplace(m_serviceLocatorClient,
        m_administrationClient, m_timeClient);
      auto definitionsClient = m_definitionsEnvironment.MakeClient(
        m_serviceLocatorClient);
      m_orderExecutionEnvironment.emplace(
        definitionsClient.LoadMarketDatabase(),
        definitionsClient.LoadDestinationDatabase(), m_serviceLocatorClient,
        m_uidClient, m_administrationClient);
      m_orderExecutionClient.emplace(m_orderExecutionEnvironment->MakeClient(
        m_serviceLocatorClient));
      auto transitionTimerFactory = [=] {
        return std::make_unique<Beam::Threading::TimerBox>(
          std::make_unique<Beam::TimeService::Tests::TestTimer>(
            boost::posix_time::seconds(1), Beam::Ref(m_timeEnvironment)));
      };
      m_riskEnvironment.emplace(m_serviceLocatorClient, m_administrationClient,
        *m_marketDataClient, *m_orderExecutionClient, transitionTimerFactory,
        m_timeClient, definitionsClient.LoadExchangeRates(),
        definitionsClient.LoadMarketDatabase(),
        definitionsClient.LoadDestinationDatabase());
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  inline TestEnvironment::~TestEnvironment() {
    Close();
  }

  inline void TestEnvironment::SetTime(boost::posix_time::ptime time) {
    m_timeEnvironment.SetTime(time);
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::AdvanceTime(
      boost::posix_time::time_duration duration) {
    m_timeEnvironment.AdvanceTime(duration);
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::Publish(MarketCode market,
      const OrderImbalance& orderImbalance) {
    PublishMarketData(market, orderImbalance);
  }

  inline void TestEnvironment::Publish(const Security& security,
      const BboQuote& bboQuote) {
    PublishMarketData(security, bboQuote);
  }

  inline void TestEnvironment::Publish(const Security& security,
      const BookQuote& bookQuote) {
    PublishMarketData(security, bookQuote);
  }

  inline void TestEnvironment::Publish(const Security& security,
      const MarketQuote& marketQuote) {
    PublishMarketData(security, marketQuote);
  }

  inline void TestEnvironment::Publish(const Security& security,
      const TimeAndSale& timeAndSale) {
    PublishMarketData(security, timeAndSale);
  }

  inline void TestEnvironment::UpdateBboPrice(const Security& security,
      Money bidPrice, Money askPrice, boost::posix_time::ptime timestamp) {
    auto quote = BboQuote(Quote(bidPrice, 100, Side::BID),
      Quote(askPrice, 100, Side::ASK), timestamp);
    Publish(security, quote);
  }

  inline void TestEnvironment::UpdateBboPrice(const Security& security,
      Money bidPrice, Money askPrice) {
    UpdateBboPrice(security, bidPrice, askPrice,
      boost::posix_time::not_a_date_time);
  }

  inline void TestEnvironment::MonitorOrderSubmissions(
      Beam::ScopedQueueWriter<const OrderExecutionService::Order*> queue) {
    auto conversionQueue = Beam::MakeConverterQueueWriter<
      OrderExecutionService::PrimitiveOrder*>(std::move(queue),
      Beam::StaticCastConverter<const OrderExecutionService::Order*>());
    auto& driver = static_cast<
      OrderExecutionService::WrapperOrderExecutionDriver<
        OrderExecutionService::Tests::MockOrderExecutionDriver>&>(
          GetOrderExecutionEnvironment().GetDriver()).GetDriver();
    driver.GetPublisher().Monitor(std::move(conversionQueue));
  }

  inline void TestEnvironment::Accept(
      const OrderExecutionService::Order& order) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitiveOrder->With([&] (auto status, auto& executionReports) {
      if(status != OrderStatus::PENDING_NEW) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order must be PENDING_NEW."));
      }
    });
    OrderExecutionService::Tests::SetOrderStatus(
      *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
      OrderStatus::NEW, m_timeEnvironment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::Reject(
      const OrderExecutionService::Order& order) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitiveOrder->With([&] (auto status, auto& executionReports) {
      if(IsTerminal(status)) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    OrderExecutionService::Tests::SetOrderStatus(
      *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
      OrderStatus::REJECTED, m_timeEnvironment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::Cancel(
      const OrderExecutionService::Order& order) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitiveOrder->With([&] (auto status, auto& executionReports) {
      if(IsTerminal(status)) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    OrderExecutionService::Tests::Cancel(
      *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
      m_timeEnvironment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::Fill(const OrderExecutionService::Order& order,
      Money price, Quantity quantity) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitiveOrder->With([&] (auto status, auto& executionReports) {
      if(IsTerminal(status)) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    OrderExecutionService::Tests::Fill(
      *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
      price, quantity, m_timeEnvironment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::Fill(const OrderExecutionService::Order& order,
      Quantity quantity) {
    Fill(order, order.GetInfo().m_fields.m_price, quantity);
  }

  inline void TestEnvironment::Update(const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException("Invalid Order specified."));
    }
    primitiveOrder->With([&] (auto status, auto& executionReports) {
      if(IsTerminal(status)) {
        BOOST_THROW_EXCEPTION(
          TestEnvironmentException("Order is already TERMINAL."));
      }
    });
    auto revisedExecutionReport = executionReport;
    if(revisedExecutionReport.m_timestamp !=
        boost::posix_time::not_a_date_time) {
      m_timeEnvironment.SetTime(revisedExecutionReport.m_timestamp);
    } else {
      revisedExecutionReport.m_timestamp = m_timeEnvironment.GetTime();
    }
    primitiveOrder->With([&] (auto status, auto& reports) {
      auto& lastReport = reports.back();
      revisedExecutionReport.m_id = lastReport.m_id;
      revisedExecutionReport.m_sequence = lastReport.m_sequence + 1;
      primitiveOrder->Update(revisedExecutionReport);
    });
    Beam::Routines::FlushPendingRoutines();
  }

  inline Beam::TimeService::Tests::TimeServiceTestEnvironment&
      TestEnvironment::GetTimeEnvironment() {
    return m_timeEnvironment;
  }

  inline Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
      TestEnvironment::GetServiceLocatorEnvironment() {
    return m_serviceLocatorEnvironment;
  }

  inline Beam::UidService::Tests::UidServiceTestEnvironment&
      TestEnvironment::GetUidEnvironment() {
    return m_uidEnvironment;
  }

  inline Beam::RegistryService::Tests::RegistryServiceTestEnvironment&
      TestEnvironment::GetRegistryEnvironment() {
    return m_registryEnvironment;
  }

  inline DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
      TestEnvironment::GetDefinitionsEnvironment() {
    return m_definitionsEnvironment;
  }

  inline AdministrationService::Tests::AdministrationServiceTestEnvironment&
      TestEnvironment::GetAdministrationEnvironment() {
    return m_administrationEnvironment;
  }

  inline MarketDataService::Tests::MarketDataServiceTestEnvironment&
      TestEnvironment::GetMarketDataEnvironment() {
    return *m_marketDataEnvironment;
  }

  inline ChartingService::Tests::ChartingServiceTestEnvironment&
      TestEnvironment::GetChartingEnvironment() {
    return *m_chartingEnvironment;
  }

  inline Compliance::Tests::ComplianceTestEnvironment&
      TestEnvironment::GetComplianceEnvironment() {
    return *m_complianceEnvironment;
  }

  inline OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
      TestEnvironment::GetOrderExecutionEnvironment() {
    return *m_orderExecutionEnvironment;
  }

  inline RiskService::Tests::RiskServiceTestEnvironment&
      TestEnvironment::GetRiskEnvironment() {
    return *m_riskEnvironment;
  }

  inline void TestEnvironment::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_riskEnvironment->Close();
    m_orderExecutionClient->Close();
    m_orderExecutionEnvironment->Close();
    m_complianceEnvironment->Close();
    m_chartingEnvironment->Close();
    m_marketDataClient->Close();
    m_marketDataEnvironment->Close();
    m_administrationClient.Close();
    m_administrationEnvironment.Close();
    m_definitionsEnvironment.Close();
    m_registryEnvironment.Close();
    m_uidClient.Close();
    m_uidEnvironment.Close();
    m_serviceLocatorClient.Close();
    m_serviceLocatorEnvironment.Close();
    m_timeClient.Close();
    m_timeEnvironment.Close();
    m_openState.Close();
  }

  template<typename Index, typename Value>
  void TestEnvironment::PublishMarketData(const Index& index,
      const Value& value) {
    if(Beam::Queries::GetTimestamp(value) !=
        boost::posix_time::not_a_date_time) {
      m_timeEnvironment.SetTime(Beam::Queries::GetTimestamp(value));
      GetMarketDataEnvironment().GetFeedClient().Publish(
        Beam::Queries::IndexedValue(value, index));
    } else {
      auto revisedValue = value;
      Beam::Queries::GetTimestamp(revisedValue) = m_timeEnvironment.GetTime();
      GetMarketDataEnvironment().GetFeedClient().Publish(
        Beam::Queries::IndexedValue(revisedValue, index));
    }
    Beam::Routines::FlushPendingRoutines();
  }
}

#endif
