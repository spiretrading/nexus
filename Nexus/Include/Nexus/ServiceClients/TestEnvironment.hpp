#ifndef NEXUS_TESTENVIRONMENT_HPP
#define NEXUS_TESTENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include "Beam/Queues/AliasQueue.hpp"
#include <Beam/Queues/ConverterWriterQueue.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/ServiceClients/TestEnvironmentException.hpp"

namespace Nexus {

  /*! \class TestEnvironment
      \brief Maintains the state needed to test a market environment.
   */
  class TestEnvironment : private boost::noncopyable {
    public:

      //! Constructs a TestEnvironment.
      TestEnvironment() = default;

      ~TestEnvironment();

      //! Sets the time.
      /*!
        \param time The time to set the environment to.
      */
      void SetTime(boost::posix_time::ptime time);

      //! Advances the time by a certain amount.
      /*!
        \param duration The amount of time to advance the environment by.
      */
      void AdvanceTime(boost::posix_time::time_duration duration);

      //! Publishes an OrderImbalance.
      /*!
        \param market The market to publish to.
        \param orderImbalance The OrderImbalance to publish.
      */
      void Publish(MarketCode market, const OrderImbalance& orderImbalance);

      //! Publishes a BboQuote.
      /*!
        \param security The Security to publish to.
        \param bboQuote The BboQuote to publish.
      */
      void Publish(const Security& security, const BboQuote& bboQuote);

      //! Publishes a BookQuote.
      /*!
        \param security The Security to publish to.
        \param bookQuote The BookQuote to publish.
      */
      void Publish(const Security& security, const BookQuote& bookQuote);

      //! Publishes a MarketQuote.
      /*!
        \param security The Security to publish to.
        \param marketQuote The MarketQuote to publish.
      */
      void Publish(const Security& security, const MarketQuote& marketQuote);

      //! Publishes a TimeAndSale.
      /*!
        \param security The Security to publish to.
        \param timeAndSale The TimeAndSale to publish.
      */
      void Publish(const Security& security, const TimeAndSale& timeAndSale);

      //! Updates the price of a BboQuote.
      /*!
        \param security The Security to update.
        \param bidPrice The updated bid price.
        \param askPrice The updated ask price.
        \param timestamp The timestamp.
      */
      void UpdateBboPrice(const Security& security, Money bidPrice,
        Money askPrice, const boost::posix_time::ptime& timestamp);

      //! Updates the price of a BboQuote.
      /*!
        \param security The Security to update.
        \param bidPrice The updated bid price.
        \param askPrice The updated ask price.
      */
      void UpdateBboPrice(const Security& security, Money bidPrice,
        Money askPrice);

      //! Monitors Orders submitted to this environment.
      /*!
        \param queue The Queue to publish submitted Orders to.
      */
      void MonitorOrderSubmissions(const std::shared_ptr<
        Beam::QueueWriter<const OrderExecutionService::Order*>>& queue);

      //! Updates a submitted order to OrderStatus NEW.
      /*!
        \param order The Order to accept.
      */
      void AcceptOrder(const OrderExecutionService::Order& order);

      //! Updates a submitted order to OrderStatus REJECTED.
      /*!
        \param order The Order to reject.
      */
      void RejectOrder(const OrderExecutionService::Order& order);

      //! Updates a submitted order to OrderStatus CANCELED.
      /*!
        \param order The Order to reject.
      */
      void CancelOrder(const OrderExecutionService::Order& order);

      //! Fills an Order.
      /*!
        \param order The Order to fill.
        \param price The price of the fill.
        \param quantity The Quantity to fill the <i>order</i> for.
      */
      void FillOrder(const OrderExecutionService::Order& order, Money price,
        Quantity quantity);

      //! Fills an Order.
      /*!
        \param order The Order to fill.
        \param quantity The Quantity to fill the <i>order</i> for.
      */
      void FillOrder(const OrderExecutionService::Order& order,
        Quantity quantity);

      //! Updates an Order.
      /*!
        \param order The Order to update.
        \param executionReport The ExecutionReport to update the <i>order</i>
               with.
      */
      void Update(const OrderExecutionService::Order& order,
        const OrderExecutionService::ExecutionReport& executionReport);

      //! Returns the TimeServiceTestEnvironment.
      Beam::TimeService::Tests::TimeServiceTestEnvironment&
        GetTimeEnvironment();

      //! Returns the ServiceLocatorTestEnvironment.
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
        GetServiceLocatorEnvironment();

      //! Returns the UidServiceTestEnvironment.
      Beam::UidService::Tests::UidServiceTestEnvironment& GetUidEnvironment();

      //! Returns the DefinitionsServiceTestEnvironment.
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
        GetDefinitionsEnvironment();

      //! Returns the AdministrationServiceTestEnvironment.
      AdministrationService::Tests::AdministrationServiceTestEnvironment&
        GetAdministrationEnvironment();

      //! Returns the MarketDataServiceTestEnvironment.
      MarketDataService::Tests::MarketDataServiceTestEnvironment&
        GetMarketDataEnvironment();

      //! Returns the OrderExecutionServiceTestEnvironment.
      OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
        GetOrderExecutionEnvironment();

      void Open();

      void Close();

    private:
      Beam::TimeService::Tests::TimeServiceTestEnvironment m_timeEnvironment;
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment
        m_serviceLocatorEnvironment;
      std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      Beam::UidService::Tests::UidServiceTestEnvironment m_uidEnvironment;
      boost::optional<
        DefinitionsService::Tests::DefinitionsServiceTestEnvironment>
        m_definitionsEnvironment;
      boost::optional<
        AdministrationService::Tests::AdministrationServiceTestEnvironment>
        m_administrationEnvironment;
      boost::optional<
        MarketDataService::Tests::MarketDataServiceTestEnvironment>
        m_marketDataEnvironment;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment>
        m_orderExecutionEnvironment;
      Beam::IO::OpenState m_openState;

      template<typename Index, typename Value>
      void PublishMarketData(const Index& index, const Value& value);
      void Shutdown();
  };

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
      Money bidPrice, Money askPrice,
      const boost::posix_time::ptime& timestamp) {
    BboQuote quote{Quote{bidPrice, 100, Side::BID},
      Quote{askPrice, 100, Side::ASK}, timestamp};
    Publish(security, quote);
  }

  inline void TestEnvironment::UpdateBboPrice(const Security& security,
      Money bidPrice, Money askPrice) {
    UpdateBboPrice(security, bidPrice, askPrice,
      boost::posix_time::not_a_date_time);
  }

  inline void TestEnvironment::MonitorOrderSubmissions(const std::shared_ptr<
      Beam::QueueWriter<const OrderExecutionService::Order*>>& queue) {
    std::shared_ptr<Beam::QueueWriter<OrderExecutionService::PrimitiveOrder*>>
      conversionQueue = Beam::MakeConverterWriterQueue<
      OrderExecutionService::PrimitiveOrder*>(Beam::MakeWeakQueue(queue),
      Beam::StaticCastConverter<const OrderExecutionService::Order*>());
    auto& driver = static_cast<
      OrderExecutionService::WrapperOrderExecutionDriver<
      OrderExecutionService::Tests::MockOrderExecutionDriver>&>(
      GetOrderExecutionEnvironment().GetDriver()).GetDriver();
    driver.GetPublisher().Monitor(
      Beam::MakeAliasQueue(conversionQueue, queue));
  }

  inline void TestEnvironment::AcceptOrder(
      const OrderExecutionService::Order& order) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException{"Invalid Order specified."});
    }
    primitiveOrder->With(
      [&] (auto status, auto& executionReports) {
        if(status != OrderStatus::PENDING_NEW) {
          BOOST_THROW_EXCEPTION(
            TestEnvironmentException{"Order must be PENDING_NEW."});
        }
      });
    OrderExecutionService::Tests::SetOrderStatus(
      *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
      OrderStatus::NEW, m_timeEnvironment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::RejectOrder(
      const OrderExecutionService::Order& order) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException{"Invalid Order specified."});
    }
    primitiveOrder->With(
      [&] (auto status, auto& executionReports) {
        if(IsTerminal(status)) {
          BOOST_THROW_EXCEPTION(
            TestEnvironmentException{"Order is already TERMINAL."});
        }
      });
    OrderExecutionService::Tests::SetOrderStatus(
      *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
      OrderStatus::REJECTED, m_timeEnvironment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::CancelOrder(
      const OrderExecutionService::Order& order) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException{"Invalid Order specified."});
    }
    primitiveOrder->With(
      [&] (auto status, auto& executionReports) {
        if(IsTerminal(status)) {
          BOOST_THROW_EXCEPTION(
            TestEnvironmentException{"Order is already TERMINAL."});
        }
      });
    OrderExecutionService::Tests::CancelOrder(
      *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
      m_timeEnvironment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::FillOrder(
      const OrderExecutionService::Order& order, Money price,
      Quantity quantity) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException{"Invalid Order specified."});
    }
    primitiveOrder->With(
      [&] (auto status, auto& executionReports) {
        if(IsTerminal(status)) {
          BOOST_THROW_EXCEPTION(
            TestEnvironmentException{"Order is already TERMINAL."});
        }
      });
    OrderExecutionService::Tests::FillOrder(
      *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
      price, quantity, m_timeEnvironment.GetTime());
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::FillOrder(
      const OrderExecutionService::Order& order, Quantity quantity) {
    FillOrder(order, order.GetInfo().m_fields.m_price, quantity);
  }

  inline void TestEnvironment::Update(
      const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException{"Invalid Order specified."});
    }
    primitiveOrder->With(
      [&] (auto status, auto& executionReports) {
        if(IsTerminal(status)) {
          BOOST_THROW_EXCEPTION(
            TestEnvironmentException{"Order is already TERMINAL."});
        }
      });
    auto revisedExecutionReport = executionReport;
    if(revisedExecutionReport.m_timestamp !=
        boost::posix_time::not_a_date_time) {
      m_timeEnvironment.SetTime(revisedExecutionReport.m_timestamp);
    } else {
      revisedExecutionReport.m_timestamp = m_timeEnvironment.GetTime();
    }
    primitiveOrder->With(
      [&] (auto status, auto& reports) {
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

  inline DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
      TestEnvironment::GetDefinitionsEnvironment() {
    return *m_definitionsEnvironment;
  }

  inline AdministrationService::Tests::AdministrationServiceTestEnvironment&
      TestEnvironment::GetAdministrationEnvironment() {
    return *m_administrationEnvironment;
  }

  inline MarketDataService::Tests::MarketDataServiceTestEnvironment&
      TestEnvironment::GetMarketDataEnvironment() {
    return *m_marketDataEnvironment;
  }

  inline OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
      TestEnvironment::GetOrderExecutionEnvironment() {
    return *m_orderExecutionEnvironment;
  }

  inline void TestEnvironment::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_timeEnvironment.Open();
      m_serviceLocatorEnvironment.Open();
      m_serviceLocatorClient = m_serviceLocatorEnvironment.BuildClient();
      m_serviceLocatorClient->SetCredentials("root", "");
      m_serviceLocatorClient->Open();
      m_uidEnvironment.Open();
      auto definitionsServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      definitionsServiceLocatorClient->SetCredentials("root", "");
      definitionsServiceLocatorClient->Open();
      m_definitionsEnvironment.emplace(
        std::move(definitionsServiceLocatorClient));
      m_definitionsEnvironment->Open();
      auto administrationServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      administrationServiceLocatorClient->SetCredentials("root", "");
      administrationServiceLocatorClient->Open();
      m_administrationEnvironment.emplace(
        std::move(administrationServiceLocatorClient));
      m_administrationEnvironment->Open();
      auto marketDataServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      marketDataServiceLocatorClient->SetCredentials("root", "");
      marketDataServiceLocatorClient->Open();
      auto marketDataAdministrationClient =
        m_administrationEnvironment->BuildClient(
        Beam::Ref(*marketDataServiceLocatorClient));
      m_marketDataEnvironment.emplace(
        std::move(marketDataServiceLocatorClient),
        std::move(marketDataAdministrationClient));
      m_marketDataEnvironment->Open();
      auto orderExecutionServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      orderExecutionServiceLocatorClient->SetCredentials("root", "");
      orderExecutionServiceLocatorClient->Open();
      auto uidClient = m_uidEnvironment.BuildClient();
      uidClient->Open();
      auto administrationClient = m_administrationEnvironment->BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      administrationClient->Open();
      m_orderExecutionEnvironment.emplace(GetDefaultMarketDatabase(),
        GetDefaultDestinationDatabase(),
        std::move(orderExecutionServiceLocatorClient), std::move(uidClient),
        std::move(administrationClient));
      m_orderExecutionEnvironment->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void TestEnvironment::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename Index, typename Value>
  void TestEnvironment::PublishMarketData(const Index& index,
      const Value& value) {
    if(Beam::Queries::GetTimestamp(value) !=
        boost::posix_time::not_a_date_time) {
      m_timeEnvironment.SetTime(Beam::Queries::GetTimestamp(value));
      GetMarketDataEnvironment().Publish(index, value);
    } else {
      auto revisedValue = value;
      Beam::Queries::GetTimestamp(revisedValue) = m_timeEnvironment.GetTime();
      GetMarketDataEnvironment().Publish(index, revisedValue);
    }
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::Shutdown() {
    m_orderExecutionEnvironment.reset();
    m_marketDataEnvironment.reset();
    m_administrationEnvironment.reset();
    m_definitionsEnvironment.reset();
    m_uidEnvironment.Close();
    m_serviceLocatorClient.reset();
    m_serviceLocatorEnvironment.Close();
    m_timeEnvironment.Close();
    m_openState.SetClosed();
  }
}

#endif
