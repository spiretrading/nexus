#ifndef OTC_LINK_SNAPSHOT_CLIENT_HPP
#define OTC_LINK_SNAPSHOT_CLIENT_HPP
#include <functional>
#include <iostream>
#include <sstream>
#include <stop_token>
#include <syncstream>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <boost/optional/optional.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkProtocolClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkSnapshot.hpp"

namespace Nexus {

  /** Concept satisfied by clients loading a channel snapshot. */
  template<typename T>
  concept IsOtcLinkSnapshotClient = requires(T& client) {
    { client.load_snapshot(std::stop_token()) } ->
      std::same_as<OtcLinkSnapshot>;
    { client.close() } -> std::same_as<void>;
  };

  /**
   * Loads a snapshot, retrying failed attempts.
   * @param retries The maximum number of retries after the initial attempt.
   * @param load Loads one attempt using a fresh snapshot client.
   * @param stop_token Cancels the load and prevents further attempts.
   */
  inline OtcLinkSnapshot load_snapshot(
    int retries, std::function<OtcLinkSnapshot (std::stop_token)> load,
    std::stop_token stop_token);

  /**
   * Loads one complete snapshot from a single snapshot feed.
   * @tparam P The protocol client receiving the snapshot channel.
   * @tparam T The timer checking snapshot inactivity.
   */
  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class OtcLinkSnapshotClient {
    public:

      /** The protocol client receiving the snapshot channel. */
      using ProtocolClient = Beam::dereference_t<P>;

      /** The timer checking snapshot inactivity. */
      using Timer = Beam::dereference_t<T>;

      /** Requests a snapshot and waits for acknowledgement. */
      using RequestFunction = std::function<void (std::stop_token)>;

      /**
       * Constructs an OtcLinkSnapshotClient.
       * @param type The spin completing this snapshot.
       * @param request Requests the snapshot, honoring cancellation.
       * @param protocol_client Receives one dedicated snapshot feed.
       * @param timer A timer with half the snapshot inactivity timeout.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<T> TF>
      OtcLinkSnapshotClient(OtcLinkSpinType type, RequestFunction request,
        PF&& protocol_client, TF&& timer);

      ~OtcLinkSnapshotClient();

      /** Loads a snapshot once, closing the snapshot feed on completion. */
      OtcLinkSnapshot load_snapshot(std::stop_token stop_token);

      /** Cancels the load and closes the snapshot feed. */
      void close();

    private:
      struct Marker {
        const char* m_name;
        OtcLinkSpinType m_type;
        std::uint32_t m_sequence;
      };
      struct Progress {
        bool m_is_acknowledged = false;
        bool m_has_activity = false;
        bool m_is_finished = false;
        int m_silent_ticks = 0;
        std::uint64_t m_packet_count = 0;
        std::uint64_t m_message_count = 0;
        std::uint64_t m_heartbeat_count = 0;
        boost::optional<std::uint32_t> m_last_packet;
        boost::optional<Marker> m_last_marker;
      };
      OtcLinkSpinType m_type;
      RequestFunction m_request;
      Beam::local_ptr_t<P> m_protocol_client;
      Beam::local_ptr_t<T> m_timer;
      Beam::Mutex m_mutex;
      bool m_is_loading;
      std::stop_source m_stop_source;
      Beam::Sync<std::exception_ptr> m_exception;
      Beam::Sync<Progress> m_progress;
      Beam::Queue<bool> m_start;
      Beam::Queue<OtcLinkSnapshot> m_snapshots;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      static const char* get_name(OtcLinkSpinType type);
      OtcLinkSnapshotClient(const OtcLinkSnapshotClient&) = delete;
      OtcLinkSnapshotClient& operator =(const OtcLinkSnapshotClient&) = delete;
      void fail(const std::exception_ptr& exception);
      void read_loop();
      void on_timer(typename Timer::Result result);
  };

  template<typename F, typename P, typename T>
  OtcLinkSnapshotClient(OtcLinkSpinType, F, P&&, T&&) ->
    OtcLinkSnapshotClient<std::remove_cvref_t<P>, std::remove_cvref_t<T>>;

  inline OtcLinkSnapshot load_snapshot(int retries,
      std::function<OtcLinkSnapshot (std::stop_token)> load,
      std::stop_token stop_token) {
    for(auto attempt = 0;; ++attempt) {
      if(stop_token.stop_requested()) {
        boost::throw_with_location(Beam::EndOfFileException());
      }
      try {
        return load(stop_token);
      } catch(const std::exception& error) {
        if(stop_token.stop_requested() || attempt >= retries) {
          throw;
        }
        std::osyncstream(std::cout) << "(snapshot_retry " << attempt + 1 <<
          ' ' << error.what() << ')' << std::endl;
      }
    }
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<P> PF, Beam::Initializes<T> TF>
  OtcLinkSnapshotClient<P, T>::OtcLinkSnapshotClient(OtcLinkSpinType type,
      RequestFunction request, PF&& protocol_client, TF&& timer)
      : m_type(type),
        m_request(std::move(request)),
        m_protocol_client(std::forward<PF>(protocol_client)),
        m_timer(std::forward<TF>(timer)),
        m_is_loading(false) {
    m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
      std::bind_front(&OtcLinkSnapshotClient::on_timer, this)));
    m_read_loop =
      Beam::spawn(std::bind_front(&OtcLinkSnapshotClient::read_loop, this));
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  OtcLinkSnapshotClient<P, T>::~OtcLinkSnapshotClient() {
    close();
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  OtcLinkSnapshot OtcLinkSnapshotClient<P, T>::load_snapshot(
      std::stop_token stop_token) {
    auto lock = std::lock_guard(m_mutex);
    if(!m_open_state.is_open() || stop_token.stop_requested()) {
      boost::throw_with_location(Beam::EndOfFileException());
    }
    if(m_is_loading) {
      boost::throw_with_location(
        Beam::IOException("OTC Link snapshot was already requested."));
    }
    m_is_loading = true;
    auto stop = std::stop_callback(stop_token, [&] {
      m_tasks.push([&] {
        fail(std::make_exception_ptr(Beam::EndOfFileException()));
      });
    });
    try {
      m_timer->start();
      m_start.push(true);
      m_request(m_stop_source.get_token());
      Beam::with(m_progress, [] (auto& progress) {
        progress.m_is_acknowledged = true;
      });
      auto snapshot = m_snapshots.pop();
      if(auto exception = m_exception.load()) {
        std::rethrow_exception(exception);
      }
      Beam::with(m_progress, [] (auto& progress) {
        progress.m_is_finished = true;
      });
      m_tasks.push([&] { m_timer->cancel(); });
      m_protocol_client->close();
      return snapshot;
    } catch(const std::exception&) {
      fail(std::current_exception());
      m_tasks.push([&] { m_timer->cancel(); });
      std::rethrow_exception(m_exception.load());
    }
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkSnapshotClient<P, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    fail(std::make_exception_ptr(Beam::EndOfFileException()));
    auto lock = std::lock_guard(m_mutex);
    m_read_loop.wait();
    m_tasks.close();
    m_tasks.wait();
    m_timer->cancel();
    m_open_state.close();
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  const char* OtcLinkSnapshotClient<P, T>::get_name(OtcLinkSpinType type) {
    if(type == OtcLinkSpinType::REFERENCE) {
      return "reference";
    } else if(type == OtcLinkSpinType::MARKET_DATA) {
      return "market_data";
    } else if(type == OtcLinkSpinType::OPENING) {
      return "opening";
    } else {
      return "unknown";
    }
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkSnapshotClient<P, T>::fail(
      const std::exception_ptr& exception) {
    Beam::with(m_progress, [] (auto& progress) {
      progress.m_is_finished = true;
    });
    Beam::with(m_exception, [&] (auto& current) {
      if(!current) {
        current = exception;
      }
    });
    m_stop_source.request_stop();
    m_start.close(exception);
    m_snapshots.close(exception);
    m_protocol_client->close();
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkSnapshotClient<P, T>::read_loop() {
    try {
      m_start.pop();
      auto snapshot = OtcLinkSnapshot();
      auto start = boost::optional<OtcLinkSpinStart>();
      auto packet_sequence = boost::optional<std::uint32_t>();
      auto is_reference_complete = false;
      while(true) {
        auto packet = m_protocol_client->read();
        auto& header = packet.get_header();
        Beam::with(m_progress, [&] (auto& progress) {
          ++progress.m_packet_count;
          progress.m_message_count += header.m_count;
          progress.m_last_packet = header.m_sequence;
          if(header.has_flag(OtcLinkHeader::Flag::HEARTBEAT)) {
            ++progress.m_heartbeat_count;
          }
        });
        if(header.has_flag(OtcLinkHeader::Flag::TEST) ||
            header.has_flag(OtcLinkHeader::Flag::REPLAY)) {
          continue;
        }
        if(header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET)) {
          if(packet_sequence) {
            boost::throw_with_location(
              Beam::IOException("OTC Link snapshot channel reset."));
          }
          continue;
        }
        if(packet_sequence) {
          if(header.m_sequence < *packet_sequence) {
            continue;
          }
          if(header.m_sequence != *packet_sequence) {
            boost::throw_with_location(
              Beam::IOException("OTC Link snapshot packet gap."));
          }
        }
        if(header.has_flag(OtcLinkHeader::Flag::HEARTBEAT)) {
          continue;
        }
        if(packet_sequence) {
          packet_sequence = header.m_sequence + 1;
        }
        auto storage = Beam::SharedBuffer();
        auto offset = std::size_t(0);
        auto has_activity = false;
        for(auto& message : packet) {
          if(message.m_type == OtcLinkSpinStart::TYPE) {
            auto next = OtcLinkSpinStart::parse(message);
            Beam::with(m_progress, [&] (auto& progress) {
              progress.m_last_marker =
                Marker("start", next.m_type, next.m_last_sequence);
            });
            if(start) {
              boost::throw_with_location(
                OtcLinkParserException("Overlapping OTC Link spins."));
            }
            if((next.m_type == m_type &&
                (m_type != OtcLinkSpinType::MARKET_DATA ||
                  is_reference_complete)) ||
                (m_type == OtcLinkSpinType::MARKET_DATA &&
                  next.m_type == OtcLinkSpinType::REFERENCE)) {
              start = next;
              packet_sequence = header.m_sequence + 1;
              has_activity = true;
            }
          } else if(message.m_type == OtcLinkSpinEnd::TYPE) {
            auto end = OtcLinkSpinEnd::parse(message);
            Beam::with(m_progress, [&] (auto& progress) {
              progress.m_last_marker =
                Marker("end", end.m_type, end.m_last_sequence);
            });
            if(start) {
              if(end.m_type != start->m_type ||
                  end.m_last_sequence != start->m_last_sequence ||
                  end.m_timestamp < start->m_timestamp) {
                boost::throw_with_location(
                  OtcLinkParserException("Mismatched OTC Link spin markers."));
              }
              if(end.m_type == m_type) {
                Beam::with(m_progress, [] (auto& progress) {
                  progress.m_has_activity = true;
                });
                snapshot.m_sequence = std::uint64_t(end.m_last_sequence) + 1;
                m_snapshots.push(std::move(snapshot));
                return;
              }
              if(end.m_type == OtcLinkSpinType::REFERENCE) {
                is_reference_complete = true;
              }
              start = boost::none;
              has_activity = true;
            }
          } else if(start) {
            if(storage.get_size() == 0) {
              auto payload = packet.get_payload();
              storage = Beam::SharedBuffer(payload.data(), payload.size());
            }
            snapshot.m_messages.push_back(storage.slice(offset,
              message.m_length));
            has_activity = true;
          }
          offset += message.m_length;
        }
        if(has_activity) {
          Beam::with(m_progress, [] (auto& progress) {
            progress.m_has_activity = true;
          });
        }
      }
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkSnapshotClient<P, T>::on_timer(typename Timer::Result result) {
    if(result == Timer::Result::CANCELED || !m_open_state.is_open()) {
      return;
    }
    auto progress = Beam::with(m_progress, [&] (auto& progress) {
      if(result == Timer::Result::EXPIRED && !progress.m_is_finished) {
        if(std::exchange(progress.m_has_activity, false)) {
          progress.m_silent_ticks = 0;
        } else {
          ++progress.m_silent_ticks;
        }
      }
      return progress;
    });
    if(progress.m_is_finished) {
      return;
    }
    if(result == Timer::Result::EXPIRED && progress.m_silent_ticks < 2) {
      m_timer->start();
      return;
    }
    auto out = std::stringstream();
    if(result == Timer::Result::EXPIRED) {
      out << "OTC Link snapshot inactivity timeout expired.";
    } else {
      out << "OTC Link snapshot timer failed.";
    }
    out << " acknowledgement=";
    if(progress.m_is_acknowledged) {
      out << "received";
    } else {
      out << "pending";
    }
    out << " packets=" << progress.m_packet_count <<
      " messages=" << progress.m_message_count <<
      " heartbeats=" << progress.m_heartbeat_count << " last_packet=";
    if(progress.m_last_packet) {
      out << *progress.m_last_packet;
    } else {
      out << "none";
    }
    out << " expected_spin=" << get_name(m_type) << " last_marker=";
    if(auto marker = progress.m_last_marker) {
      out << marker->m_name << '/' << get_name(marker->m_type) << '/' <<
        marker->m_sequence;
    } else {
      out << "none";
    }
    fail(std::make_exception_ptr(Beam::IOException(out.str())));
  }
}

#endif
