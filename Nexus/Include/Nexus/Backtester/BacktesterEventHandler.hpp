#ifndef NEXUS_BACKTESTEREVENTHANDLER_HPP
#define NEXUS_BACKTESTEREVENTHANDLER_HPP
#include <deque>
#include <unordered_set>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/LockRelease.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/iterator_range.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEvent.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {

  /*! \class BacktesterEventHandler
      \brief Implements an event loop to handle BacktesterEvents.
   */
  class BacktesterEventHandler : private boost::noncopyable {
    public:

      //! Constructs a BacktesterEventHandler.
      /*!
        \param startTime The starting point of the backtester.
      */
      BacktesterEventHandler(boost::posix_time::ptime startTime);

      //! Constructs a BacktesterEventHandler.
      /*!
        \param startTime The starting point of the backtester.
        \param endTime The time to stop backtesting.
      */
      BacktesterEventHandler(boost::posix_time::ptime startTime,
        boost::posix_time::ptime endTime);

      ~BacktesterEventHandler();

      //! Returns the start time.
      boost::posix_time::ptime GetStartTime() const;

      //! Returns the end time.
      boost::posix_time::ptime GetEndTime() const;

      //! Adds an event to be handled.
      /*!
        \param event The event to handle.
      */
      void Add(std::shared_ptr<BacktesterEvent> event);

      //! Adds a list of events to be handled.
      /*!
        \param events The list of events to handle.
      */
      void Add(std::vector<std::shared_ptr<BacktesterEvent>> events);

      //! Returns the TestEnvironment used.
      const TestEnvironment& GetTestEnvironment() const;

      //! Returns the TestEnvironment used.
      TestEnvironment& GetTestEnvironment();

      void Open();

      void Close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      boost::posix_time::ptime m_startTime;
      boost::posix_time::ptime m_endTime;
      TestEnvironment m_testEnvironment;
      std::deque<std::shared_ptr<BacktesterEvent>> m_events;
      Beam::Threading::ConditionVariable m_eventAvailableCondition;
      Beam::Routines::RoutineHandler m_eventLoopRoutine;
      Beam::IO::OpenState m_openState;

      void EventLoop();
      void Shutdown();
  };

  inline BacktesterEventHandler::BacktesterEventHandler(
      boost::posix_time::ptime startTime)
      : BacktesterEventHandler{std::move(startTime),
          boost::posix_time::pos_infin} {}

  inline BacktesterEventHandler::BacktesterEventHandler(
      boost::posix_time::ptime startTime, boost::posix_time::ptime endTime)
      : m_startTime{std::move(startTime)},
        m_endTime{std::move(endTime)} {}

  inline BacktesterEventHandler::~BacktesterEventHandler() {
    Close();
  }

  inline boost::posix_time::ptime BacktesterEventHandler::
      GetStartTime() const {
    return m_startTime;
  }

  inline boost::posix_time::ptime BacktesterEventHandler::GetEndTime() const {
    return m_endTime;
  }

  inline void BacktesterEventHandler::Add(
      std::shared_ptr<BacktesterEvent> event) {
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
      auto insertIterator = std::lower_bound(m_events.begin(), m_events.end(),
        event,
        [] (auto& lhs, auto& rhs) {
          return lhs->GetTimestamp() < rhs->GetTimestamp();
        });
      m_events.insert(insertIterator, std::move(event));
    }
    m_eventAvailableCondition.notify_one();
  }

  inline void BacktesterEventHandler::Add(
      std::vector<std::shared_ptr<BacktesterEvent>> events) {
    if(events.empty()) {
      return;
    }
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
      for(auto& event : events) {
        auto insertIterator = std::lower_bound(m_events.begin(),
          m_events.end(), event,
          [] (auto& lhs, auto& rhs) {
            return lhs->GetTimestamp() < rhs->GetTimestamp();
          });
        m_events.insert(insertIterator, std::move(event));
      }
    }
    m_eventAvailableCondition.notify_one();
  }

  inline const TestEnvironment& BacktesterEventHandler::
      GetTestEnvironment() const {
    return m_testEnvironment;
  }

  inline TestEnvironment& BacktesterEventHandler::GetTestEnvironment() {
    return m_testEnvironment;
  }

  inline void BacktesterEventHandler::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_testEnvironment.SetTime(m_startTime);
      m_testEnvironment.Open();
      m_eventLoopRoutine = Beam::Routines::Spawn(
        std::bind(&BacktesterEventHandler::EventLoop, this));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void BacktesterEventHandler::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterEventHandler::EventLoop() {
    while(true) {
      std::shared_ptr<BacktesterEvent> event;
      {
        boost::unique_lock<Beam::Threading::Mutex> lock{m_mutex};
        while(m_openState.IsOpen() && m_events.empty()) {
          m_eventAvailableCondition.wait(lock);
        }
        if(!m_openState.IsOpen()) {
          return;
        }
        {
          auto release = Beam::Threading::Release(lock);
          Beam::Routines::FlushPendingRoutines();
        }
        event = m_events.front();
        m_events.pop_front();
        if(event->GetTimestamp() != boost::posix_time::neg_infin) {
          m_testEnvironment.SetTime(event->GetTimestamp());
        }
      }
      event->Execute();
    }
  }

  inline void BacktesterEventHandler::Shutdown() {
    m_eventAvailableCondition.notify_one();
    m_eventLoopRoutine.Wait();
    m_testEnvironment.Close();
    m_openState.SetClosed();
  }
}

#endif
