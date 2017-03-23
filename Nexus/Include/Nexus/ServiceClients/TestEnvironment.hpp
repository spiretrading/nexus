#ifndef NEXUS_TESTENVIRONMENT_HPP
#define NEXUS_TESTENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include <Beam/Utilities/SynchronizedList.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceInstance.hpp"

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

      //! Returns the ServiceLocatorTestInstance.
      Beam::ServiceLocator::Tests::ServiceLocatorTestInstance&
        GetServiceLocatorInstance();

      //! Returns the UidServiceTestInstance.
      Beam::UidService::Tests::UidServiceTestInstance& GetUidInstance();

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
    boost::unique_lock<Beam::Threading::Mutex> lock{m_timeMutex};
    LockedSetTime(time, lock);
  }

  inline void TestEnvironment::AdvanceTime(
      boost::posix_time::time_duration duration) {
    boost::unique_lock<Beam::Threading::Mutex> lock{m_timeMutex};
    LockedSetTime(m_currentTime + duration, lock);
  }

  inline void TestEnvironment::Update(const Security& security,
      const BboQuote& bboQuote) {
    boost::unique_lock<Beam::Threading::Mutex> lock{m_timeMutex};
    if(bboQuote.m_timestamp != boost::posix_time::not_a_date_time) {
      LockedSetTime(bboQuote.m_timestamp, lock);
      GetMarketDataInstance().SetBbo(security, bboQuote);
    } else {
      auto revisedBboQuote = bboQuote;
      revisedBboQuote.m_timestamp = m_currentTime;
      GetMarketDataInstance().SetBbo(security, bboQuote);
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
