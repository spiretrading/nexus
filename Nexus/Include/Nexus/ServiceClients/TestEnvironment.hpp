#ifndef NEXUS_TESTENVIRONMENT_HPP
#define NEXUS_TESTENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/ConverterWriterQueue.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include <Beam/Utilities/SynchronizedList.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestInstance.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/ServiceClients/TestEnvironmentException.hpp"

namespace Nexus {
  class TestTimeClient;
  class TestTimer;
  void Trigger(TestTimer& timer);

  /*! \class TestEnvironment
      \brief Maintains the state needed to test a market environment.
   */
  class TestEnvironment : private boost::noncopyable {
    public:

      //! Constructs a TestEnvironment.
      TestEnvironment();

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

      //! Updates a BboQuote.
      /*!
        \param security The Security to update.
        \param bboQuote The updated BboQuote.
      */
      void Update(const Security& security, const BboQuote& bboQuote);

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

      //! Returns the ServiceLocatorTestInstance.
      Beam::ServiceLocator::Tests::ServiceLocatorTestInstance&
        GetServiceLocatorInstance();

      //! Returns the UidServiceTestInstance.
      Beam::UidService::Tests::UidServiceTestInstance& GetUidInstance();

      //! Returns the DefinitionsServiceTestInstance.
      DefinitionsService::Tests::DefinitionsServiceTestInstance&
        GetDefinitionsInstance();

      //! Returns the AdministrationServiceTestInstance.
      AdministrationService::Tests::AdministrationServiceTestInstance&
        GetAdministrationInstance();

      //! Returns the MarketDataServiceTestInstance.
      MarketDataService::Tests::MarketDataServiceTestInstance&
        GetMarketDataInstance();

      //! Returns the OrderExecutionServiceTestInstance.
      OrderExecutionService::Tests::OrderExecutionServiceTestInstance&
        GetOrderExecutionInstance();

      void Open();

      void Close();

    private:
      struct TimerEntry {
        TestTimer* m_timer;
        boost::posix_time::time_duration m_timeRemaining;
      };
      friend class TestTimeClient;
      friend class TestTimer;
      Beam::Threading::Mutex m_timeMutex;
      boost::posix_time::ptime m_currentTime;
      boost::posix_time::time_duration m_nextTrigger;
      Beam::ServiceLocator::Tests::ServiceLocatorTestInstance
        m_serviceLocatorInstance;
      std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      Beam::UidService::Tests::UidServiceTestInstance m_uidInstance;
      boost::optional<DefinitionsService::Tests::DefinitionsServiceTestInstance>
        m_definitionsInstance;
      boost::optional<
        AdministrationService::Tests::AdministrationServiceTestInstance>
        m_administrationInstance;
      boost::optional<MarketDataService::Tests::MarketDataServiceTestInstance>
        m_marketDataInstance;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestInstance>
        m_orderExecutionInstance;
      Beam::SynchronizedVector<TestTimeClient*> m_timeClients;
      Beam::SynchronizedVector<TimerEntry> m_timers;
      Beam::SynchronizedVector<std::shared_ptr<void>> m_converters;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      
      void LockedSetTime(boost::posix_time::ptime time,
        boost::unique_lock<Beam::Threading::Mutex>& lock);
      void Add(TestTimeClient* timeClient);
      void Remove(TestTimeClient* timeClient);
      void Add(TestTimer* timer);
      void Remove(TestTimer* timer);
  };

  inline TestEnvironment::TestEnvironment()
      : m_nextTrigger{boost::posix_time::pos_infin} {}

  inline TestEnvironment::~TestEnvironment() {
    Close();
  }

  inline void TestEnvironment::SetTime(boost::posix_time::ptime time) {
    if(time.is_special()) {
      BOOST_THROW_EXCEPTION(TestEnvironmentException{"Invalid date/time."});
    }
    boost::unique_lock<Beam::Threading::Mutex> lock{m_timeMutex};
    LockedSetTime(time, lock);
  }

  inline void TestEnvironment::AdvanceTime(
      boost::posix_time::time_duration duration) {
    boost::unique_lock<Beam::Threading::Mutex> lock{m_timeMutex};
    if(m_currentTime == boost::posix_time::not_a_date_time) {
      m_currentTime = boost::posix_time::ptime{
        boost::gregorian::date{2016, 8, 14}, boost::posix_time::seconds(0)};
    }
    LockedSetTime(m_currentTime + duration, lock);
  }

  inline void TestEnvironment::Update(const Security& security,
      const BboQuote& bboQuote) {
    {
      boost::unique_lock<Beam::Threading::Mutex> lock{m_timeMutex};
      if(bboQuote.m_timestamp != boost::posix_time::not_a_date_time) {
        LockedSetTime(bboQuote.m_timestamp, lock);
        GetMarketDataInstance().SetBbo(security, bboQuote);
      } else {
        auto revisedBboQuote = bboQuote;
        revisedBboQuote.m_timestamp = m_currentTime;
        GetMarketDataInstance().SetBbo(security, revisedBboQuote);
      }
    }
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironment::MonitorOrderSubmissions(const std::shared_ptr<
      Beam::QueueWriter<const OrderExecutionService::Order*>>& queue) {
    auto weakQueue = Beam::MakeWeakQueue(queue);
    auto conversionQueue = Beam::MakeConverterWriterQueue<
      OrderExecutionService::PrimitiveOrder*>(weakQueue,
      Beam::StaticCastConverter<const OrderExecutionService::Order*>());
    GetOrderExecutionInstance().GetDriver().GetPublisher().Monitor(
      conversionQueue);
    m_converters.PushBack(conversionQueue);
  }

  inline void TestEnvironment::AcceptOrder(
      const OrderExecutionService::Order& order) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException{"Invalid Order specified."});
    }
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_timeMutex};
      primitiveOrder->With(
        [&] (auto status, auto& executionReports) {
          if(status != OrderStatus::PENDING_NEW) {
            BOOST_THROW_EXCEPTION(
              TestEnvironmentException{"Order must be PENDING_NEW."});
          }
        });
      OrderExecutionService::Tests::SetOrderStatus(
        *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
        OrderStatus::NEW, m_currentTime);
    }
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
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_timeMutex};
      primitiveOrder->With(
        [&] (auto status, auto& executionReports) {
          if(IsTerminal(status)) {
            BOOST_THROW_EXCEPTION(
              TestEnvironmentException{"Order is already TERMINAL."});
          }
        });
      OrderExecutionService::Tests::SetOrderStatus(
        *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
        OrderStatus::REJECTED, m_currentTime);
    }
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
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_timeMutex};
      primitiveOrder->With(
        [&] (auto status, auto& executionReports) {
          if(IsTerminal(status)) {
            BOOST_THROW_EXCEPTION(
              TestEnvironmentException{"Order is already TERMINAL."});
          }
        });
      OrderExecutionService::Tests::CancelOrder(
        *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
        m_currentTime);
    }
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
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_timeMutex};
      primitiveOrder->With(
        [&] (auto status, auto& executionReports) {
          if(IsTerminal(status)) {
            BOOST_THROW_EXCEPTION(
              TestEnvironmentException{"Order is already TERMINAL."});
          }
        });
      OrderExecutionService::Tests::FillOrder(
        *const_cast<OrderExecutionService::PrimitiveOrder*>(primitiveOrder),
        price, quantity, m_currentTime);
    }
    Beam::Routines::FlushPendingRoutines();
  }

  void TestEnvironment::FillOrder(const OrderExecutionService::Order& order,
      Quantity quantity) {
    FillOrder(order, order.GetInfo().m_fields.m_price, quantity);
  }

  void TestEnvironment::Update(const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto primitiveOrder = const_cast<OrderExecutionService::PrimitiveOrder*>(
      dynamic_cast<const OrderExecutionService::PrimitiveOrder*>(&order));
    if(primitiveOrder == nullptr) {
      BOOST_THROW_EXCEPTION(
        TestEnvironmentException{"Invalid Order specified."});
    }
    {
      boost::unique_lock<Beam::Threading::Mutex> lock{m_timeMutex};
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
        LockedSetTime(revisedExecutionReport.m_timestamp, lock);
      } else {
        revisedExecutionReport.m_timestamp = m_currentTime;
      }
      primitiveOrder->With(
        [&] (auto status, auto& reports) {
          auto& lastReport = reports.back();
          revisedExecutionReport.m_id = lastReport.m_id;
          revisedExecutionReport.m_sequence = lastReport.m_sequence + 1;
          primitiveOrder->Update(revisedExecutionReport);
        });
    }
    Beam::Routines::FlushPendingRoutines();
  }

  inline Beam::ServiceLocator::Tests::ServiceLocatorTestInstance&
      TestEnvironment::GetServiceLocatorInstance() {
    return m_serviceLocatorInstance;
  }

  inline Beam::UidService::Tests::UidServiceTestInstance&
      TestEnvironment::GetUidInstance() {
    return m_uidInstance;
  }

  inline DefinitionsService::Tests::DefinitionsServiceTestInstance&
      TestEnvironment::GetDefinitionsInstance() {
    return *m_definitionsInstance;
  }

  inline AdministrationService::Tests::AdministrationServiceTestInstance&
      TestEnvironment::GetAdministrationInstance() {
    return *m_administrationInstance;
  }

  inline MarketDataService::Tests::MarketDataServiceTestInstance&
      TestEnvironment::GetMarketDataInstance() {
    return *m_marketDataInstance;
  }

  inline OrderExecutionService::Tests::OrderExecutionServiceTestInstance&
      TestEnvironment::GetOrderExecutionInstance() {
    return *m_orderExecutionInstance;
  }

  inline void TestEnvironment::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceLocatorInstance.Open();
      m_serviceLocatorClient = m_serviceLocatorInstance.BuildClient();
      m_serviceLocatorClient->SetCredentials("root", "");
      m_serviceLocatorClient->Open();
      m_uidInstance.Open();
      auto definitionsServiceLocatorClient =
        m_serviceLocatorInstance.BuildClient();
      definitionsServiceLocatorClient->SetCredentials("root", "");
      definitionsServiceLocatorClient->Open();
      m_definitionsInstance.emplace(std::move(definitionsServiceLocatorClient));
      m_definitionsInstance->Open();
      auto administrationServiceLocatorClient =
        m_serviceLocatorInstance.BuildClient();
      administrationServiceLocatorClient->SetCredentials("root", "");
      administrationServiceLocatorClient->Open();
      m_administrationInstance.emplace(
        std::move(administrationServiceLocatorClient));
      m_administrationInstance->Open();
      auto marketDataServiceLocatorClient =
        m_serviceLocatorInstance.BuildClient();
      marketDataServiceLocatorClient->SetCredentials("root", "");
      marketDataServiceLocatorClient->Open();
      m_marketDataInstance.emplace(std::move(marketDataServiceLocatorClient));
      m_marketDataInstance->Open();
      auto orderExecutionServiceLocatorClient =
        m_serviceLocatorInstance.BuildClient();
      orderExecutionServiceLocatorClient->SetCredentials("root", "");
      orderExecutionServiceLocatorClient->Open();
      auto uidClient = m_uidInstance.BuildClient();
      uidClient->Open();
      auto administrationClient = m_administrationInstance->BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      administrationClient->Open();
      m_orderExecutionInstance.emplace(
        std::move(orderExecutionServiceLocatorClient),
        std::move(uidClient), std::move(administrationClient));
      m_orderExecutionInstance->Open();
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

  inline void TestEnvironment::Shutdown() {
    m_orderExecutionInstance.reset();
    m_marketDataInstance.reset();
    m_administrationInstance.reset();
    m_definitionsInstance.reset();
    m_uidInstance.Close();
    m_serviceLocatorClient.reset();
    m_serviceLocatorInstance.Close();
    m_openState.SetClosed();
  }

  inline void TestEnvironment::LockedSetTime(boost::posix_time::ptime time,
      boost::unique_lock<Beam::Threading::Mutex>& lock) {
    if(m_currentTime != boost::posix_time::not_a_date_time &&
        m_currentTime >= time) {
      return;
    }
    auto delta =
      [&] {
        if(m_currentTime == boost::posix_time::not_a_date_time) {
          return boost::posix_time::time_duration{};
        } else {
          return time - m_currentTime;
        }
      }();
    while(delta > m_nextTrigger) {
      delta -= m_nextTrigger;
      LockedSetTime(m_currentTime + m_nextTrigger, lock);
    }
    m_nextTrigger -= delta;
    if(m_nextTrigger <= boost::posix_time::seconds(0)) {
      m_nextTrigger = boost::posix_time::pos_infin;
    }
    m_currentTime = time;
    m_timeClients.ForEach(
      [&] (auto& timeClient) {
        timeClient->SetTime(time);
      });
    std::vector<TestTimer*> expiredTimers;
    m_timers.With(
      [&] (auto& timers) {
        auto i = timers.begin();
        while(i != timers.end()) {
          auto& timer = *i;
          timer.m_timeRemaining -= delta;
          if(timer.m_timeRemaining <= boost::posix_time::seconds(0)) {
            expiredTimers.push_back(timer.m_timer);
            i = timers.erase(i);
          } else {
            m_nextTrigger = std::min(m_nextTrigger, timer.m_timeRemaining);
            ++i;
          }
        }
      });
    {
      auto release = Beam::Threading::Release(lock);
      for(auto& expiredTimer : expiredTimers) {
        Trigger(*expiredTimer);
      }
      Beam::Routines::FlushPendingRoutines();
    }
  }

  inline void TestEnvironment::Remove(TestTimeClient* timeClient) {
    m_timeClients.Remove(timeClient);
  }

  inline void TestEnvironment::Remove(TestTimer* timer) {
    m_timers.RemoveIf(
      [&] (auto& entry) {
        return entry.m_timer == timer;
      });
  }
}

#endif
