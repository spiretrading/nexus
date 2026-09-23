#ifndef OTC_LINK_SNAPSHOT_CLIENT_HPP
#define OTC_LINK_SNAPSHOT_CLIENT_HPP
#include <functional>
#include <stop_token>
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
   * Loads one complete snapshot from a single snapshot feed.
   * @tparam P The protocol client receiving the snapshot channel.
   * @tparam T The timer bounding the acknowledgement and snapshot.
   */
  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class OtcLinkSnapshotClient {
    public:

      /** The protocol client receiving the snapshot channel. */
      using ProtocolClient = Beam::dereference_t<P>;

      /** The timer bounding the acknowledgement and snapshot. */
      using Timer = Beam::dereference_t<T>;

      /** Requests a snapshot and waits for acknowledgement. */
      using RequestFunction = std::function<void (std::stop_token)>;

      /**
       * Constructs an OtcLinkSnapshotClient.
       * @param type The spin completing this snapshot. Reference spins
       *        preceding a market-data spin are included.
       * @param request Requests the snapshot, honoring cancellation.
       * @param protocol_client Receives one dedicated snapshot feed.
       * @param timer The timer bounding the complete load.
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
      OtcLinkSpinType m_type;
      RequestFunction m_request;
      Beam::local_ptr_t<P> m_protocol_client;
      Beam::local_ptr_t<T> m_timer;
      Beam::Mutex m_mutex;
      bool m_is_loading;
      std::stop_source m_stop_source;
      Beam::Sync<std::exception_ptr> m_exception;
      Beam::Queue<bool> m_start;
      Beam::Queue<OtcLinkSnapshot> m_snapshots;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      OtcLinkSnapshotClient(const OtcLinkSnapshotClient&) = delete;
      OtcLinkSnapshotClient& operator =(const OtcLinkSnapshotClient&) = delete;
      void fail(const std::exception_ptr& exception);
      void read_loop();
      void on_timer(typename Timer::Result result);
  };

  template<typename F, typename P, typename T>
  OtcLinkSnapshotClient(OtcLinkSpinType, F, P&&, T&&) ->
    OtcLinkSnapshotClient<std::remove_cvref_t<P>, std::remove_cvref_t<T>>;

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
      auto snapshot = m_snapshots.pop();
      if(auto exception = m_exception.load()) {
        std::rethrow_exception(exception);
      }
      m_timer->cancel();
      m_protocol_client->close();
      return snapshot;
    } catch(const std::exception&) {
      fail(std::current_exception());
      m_timer->cancel();
      throw;
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
    m_timer->cancel();
    m_read_loop.wait();
    m_tasks.close();
    m_tasks.wait();
    m_open_state.close();
  }

  template<typename P, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkSnapshotClient<P, T>::fail(
      const std::exception_ptr& exception) {
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
      while(true) {
        auto packet = m_protocol_client->read();
        auto& header = packet.get_header();
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
        for(auto& message : packet) {
          if(message.m_type == OtcLinkSpinStart::TYPE) {
            auto next = OtcLinkSpinStart::parse(message);
            if(start) {
              boost::throw_with_location(
                OtcLinkParserException("Overlapping OTC Link spins."));
            }
            if(next.m_type == m_type ||
                (m_type == OtcLinkSpinType::MARKET_DATA &&
                  next.m_type == OtcLinkSpinType::REFERENCE)) {
              start = next;
              packet_sequence = header.m_sequence + 1;
            }
          } else if(message.m_type == OtcLinkSpinEnd::TYPE) {
            auto end = OtcLinkSpinEnd::parse(message);
            if(start) {
              if(end.m_type != start->m_type ||
                  end.m_last_sequence != start->m_last_sequence ||
                  end.m_timestamp < start->m_timestamp) {
                boost::throw_with_location(
                  OtcLinkParserException("Mismatched OTC Link spin markers."));
              }
              if(end.m_type == m_type) {
                snapshot.m_sequence = std::uint64_t(end.m_last_sequence) + 1;
                m_snapshots.push(std::move(snapshot));
                return;
              }
              start = boost::none;
            }
          } else if(start) {
            if(storage.get_size() == 0) {
              auto payload = packet.get_payload();
              storage = Beam::SharedBuffer(payload.data(), payload.size());
            }
            snapshot.m_messages.push_back(storage.slice(offset,
              message.m_length));
          }
          offset += message.m_length;
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
    if(result != Timer::Result::CANCELED && m_open_state.is_open()) {
      fail(std::make_exception_ptr(
        Beam::IOException("OTC Link snapshot deadline failed or expired.")));
    }
  }
}

#endif
