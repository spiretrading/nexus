#ifndef NEXUS_BACKTESTER_EVENT_HANDLER_HPP
#define NEXUS_BACKTESTER_EVENT_HANDLER_HPP
#include <deque>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/LockRelease.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/iterator_range.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEvent.hpp"

namespace Nexus {

  /** Implements an event loop to handle BacktesterEvents. */
  class BacktesterEventHandler : private boost::noncopyable {
    public:

      /**
       * Constructs a BacktesterEventHandler.
       * @param startTime The starting point of the backtester.
       */
      BacktesterEventHandler(boost::posix_time::ptime startTime);

      /**
       * Constructs a BacktesterEventHandler.
       * @param startTime The starting point of the backtester.
       * @param endTime The time to stop backtesting.
       */
      BacktesterEventHandler(boost::posix_time::ptime startTime,
        boost::posix_time::ptime endTime);

      ~BacktesterEventHandler();

      /** Returns the start time. */
      boost::posix_time::ptime GetStartTime() const;

      /** Returns the end time. */
      boost::posix_time::ptime GetEndTime() const;

      /** Returns the current time. */
      boost::posix_time::ptime GetTime() const;

      /**
       * Adds an event to be handled.
       * @param event The event to handle.
       */
      void Add(std::shared_ptr<BacktesterEvent> event);

      /**
       * Adds a list of events to be handled.
       * @param events The list of events to handle.
       */
      void Add(std::vector<std::shared_ptr<BacktesterEvent>> events);

      void Close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      boost::posix_time::ptime m_startTime;
      boost::posix_time::ptime m_endTime;
      Beam::TimeService::Tests::TimeServiceTestEnvironment m_timeEnvironment;
      std::deque<std::shared_ptr<BacktesterEvent>> m_events;
      Beam::Threading::ConditionVariable m_eventAvailableCondition;
      Beam::Routines::RoutineHandler m_eventLoopRoutine;
      Beam::IO::OpenState m_openState;

      void EventLoop();
      void Shutdown();
  };

  inline BacktesterEventHandler::BacktesterEventHandler(
    boost::posix_time::ptime startTime)
    : BacktesterEventHandler(std::move(startTime),
        boost::posix_time::pos_infin) {}

  inline BacktesterEventHandler::BacktesterEventHandler(
      boost::posix_time::ptime startTime, boost::posix_time::ptime endTime)
      : m_startTime(std::move(startTime)),
        m_endTime(std::move(endTime)) {
    m_openState.SetOpening();
    try {
      m_timeEnvironment.SetTime(m_startTime);
      m_eventLoopRoutine = Beam::Routines::Spawn(
        std::bind(&BacktesterEventHandler::EventLoop, this));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline BacktesterEventHandler::~BacktesterEventHandler() {
    Close();
  }

  inline boost::posix_time::ptime BacktesterEventHandler::GetStartTime() const {
    return m_startTime;
  }

  inline boost::posix_time::ptime BacktesterEventHandler::GetEndTime() const {
    return m_endTime;
  }

  inline boost::posix_time::ptime BacktesterEventHandler::GetTime() const {
    return m_timeEnvironment.GetTime();
  }

  inline void BacktesterEventHandler::Add(
      std::shared_ptr<BacktesterEvent> event) {
    {
      auto lock = boost::lock_guard(m_mutex);
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
      auto lock = boost::lock_guard(m_mutex);
      for(auto& event : events) {
        auto insertIterator = std::lower_bound(m_events.begin(), m_events.end(),
          event,
          [] (auto& lhs, auto& rhs) {
            return lhs->GetTimestamp() < rhs->GetTimestamp();
          });
        m_events.insert(insertIterator, std::move(event));
      }
    }
    m_eventAvailableCondition.notify_one();
  }

  inline void BacktesterEventHandler::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterEventHandler::EventLoop() {
    while(true) {
      auto event = std::shared_ptr<BacktesterEvent>();
      {
        auto lock = boost::unique_lock(m_mutex);
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
          m_timeEnvironment.SetTime(event->GetTimestamp());
        }
      }
      event->Execute();
    }
  }

  inline void BacktesterEventHandler::Shutdown() {
    m_eventAvailableCondition.notify_one();
    m_eventLoopRoutine.Wait();
    m_timeEnvironment.Close();
    Beam::Routines::FlushPendingRoutines();
    m_openState.SetClosed();
  }
}

#endif
