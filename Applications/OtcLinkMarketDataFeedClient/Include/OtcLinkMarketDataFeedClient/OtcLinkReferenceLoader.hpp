#ifndef OTC_LINK_REFERENCE_LOADER_HPP
#define OTC_LINK_REFERENCE_LOADER_HPP
#include <functional>
#include <iostream>
#include <stop_token>
#include <syncstream>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"

namespace Nexus {

  /**
   * Refreshes OTC security definitions at each scheduled pre-open.
   * @tparam R The source of the current time.
   * @tparam T The timer checking for scheduled refreshes.
   */
  template<typename R, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class OtcLinkReferenceLoader {
    public:

      /** Loads and installs a reference snapshot, honoring cancellation. */
      using LoadFunction = std::function<void (std::stop_token)>;

      /**
       * Constructs a reference loader after the startup snapshot is loaded.
       * @param schedule The schedule defining OTC pre-open events.
       * @param time_client The source of the current time.
       * @param timer The timer checking for scheduled refreshes.
       * @param load Loads and installs fresh security definitions.
       */
      template<Beam::Initializes<R> RF, Beam::Initializes<T> TF>
      OtcLinkReferenceLoader(TradingSchedule schedule, RF&& time_client,
        TF&& timer, LoadFunction load);

      ~OtcLinkReferenceLoader();

      /** Cancels a pending refresh and stops scheduling snapshots. */
      void close();

    private:
      using Timer = Beam::dereference_t<T>;
      TradingSchedule m_schedule;
      Beam::local_ptr_t<R> m_time_client;
      Beam::local_ptr_t<T> m_timer;
      LoadFunction m_load;
      boost::posix_time::ptime m_timestamp;
      std::stop_source m_stop_source;
      Beam::RoutineTaskQueue m_tasks;
      Beam::OpenState m_open_state;

      OtcLinkReferenceLoader(const OtcLinkReferenceLoader&) = delete;
      OtcLinkReferenceLoader& operator =(const OtcLinkReferenceLoader&) =
        delete;
      void on_timer(typename Timer::Result result);
  };

  template<typename R, typename T, typename F>
  OtcLinkReferenceLoader(TradingSchedule, R&&, T&&, F) ->
    OtcLinkReferenceLoader<std::remove_cvref_t<R>, std::remove_cvref_t<T>>;

  template<typename R, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<R> RF, Beam::Initializes<T> TF>
  OtcLinkReferenceLoader<R, T>::OtcLinkReferenceLoader(TradingSchedule schedule,
      RF&& time_client, TF&& timer, LoadFunction load)
      : m_schedule(std::move(schedule)),
        m_time_client(std::forward<RF>(time_client)),
        m_timer(std::forward<TF>(timer)),
        m_load(std::move(load)),
        m_timestamp(m_time_client->get_time()) {
    try {
      m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
        std::bind_front(&OtcLinkReferenceLoader::on_timer, this)));
      m_timer->start();
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename R, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  OtcLinkReferenceLoader<R, T>::~OtcLinkReferenceLoader() {
    close();
  }

  template<typename R, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkReferenceLoader<R, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_stop_source.request_stop();
    m_tasks.close();
    m_tasks.wait();
    m_timer->cancel();
    m_open_state.close();
  }

  template<typename R, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkReferenceLoader<R, T>::on_timer(typename Timer::Result result) {
    if(!m_open_state.is_open() || result == Timer::Result::CANCELED) {
      return;
    }
    try {
      auto timestamp = m_time_client->get_time();
      auto previous = m_timestamp;
      m_timestamp = std::max(m_timestamp, timestamp);
      auto events = m_schedule.find(timestamp, Venues::OTCM,
        [&] (const auto& event) {
          return event.m_code == "PRE_OPEN" &&
            event.m_timestamp > previous && event.m_timestamp <= timestamp;
        });
      if(!events.empty()) {
        m_load(m_stop_source.get_token());
      }
    } catch(const std::exception& e) {
      if(m_open_state.is_open()) {
        std::osyncstream(std::cout) << "(reference_failed " << m_timestamp <<
          ' ' << e.what() << ')' << std::endl;
      }
    }
    if(m_open_state.is_open()) {
      m_timer->start();
    }
  }
}

#endif
