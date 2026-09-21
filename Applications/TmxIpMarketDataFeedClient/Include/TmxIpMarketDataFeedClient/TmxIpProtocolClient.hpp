#ifndef TMX_IP_PROTOCOL_CLIENT_HPP
#define TMX_IP_PROTOCOL_CLIENT_HPP
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpPacket.hpp"

namespace Nexus {

  /** Concept satisfied by types delivering the packets of a TMX IP feed. */
  template<typename T>
  concept IsTmxIpProtocolClient = requires(T& t) {
    { t.read() } -> std::same_as<TmxIpPacket>;
    { t.read(std::declval<Beam::Out<boost::posix_time::ptime>>()) } ->
      std::same_as<TmxIpPacket>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Reads the packets delivered by a TMX IP multicast feed.
   * @tparam C The type of Channel delivering one complete datagram per read.
   * @tparam R The type of TimeClient providing ingress timestamps in UTC.
   */
  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  class TmxIpProtocolClient {
    public:

      /** The type of channel receiving the feed. */
      using Channel = Beam::dereference_t<C>;

      /** The type of time client providing ingress timestamps. */
      using TimeClient = Beam::dereference_t<R>;

      /**
       * Constructs a TmxIpProtocolClient.
       * @param channel The channel receiving the feed.
       * @param time_client The time client providing ingress timestamps in UTC.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<R> RF>
      TmxIpProtocolClient(CF&& channel, RF&& time_client);

      ~TmxIpProtocolClient();

      /**
       * Reads the next packet from the feed.
       * The returned views are valid until the next read or destruction.
       */
      TmxIpPacket read();

      /**
       * Reads the next packet and its ingress timestamp in UTC.
       * The returned views are valid until the next read or destruction.
       * @param timestamp Receives the time the datagram was read.
       */
      TmxIpPacket read(Beam::Out<boost::posix_time::ptime> timestamp);

      void close();

    private:
      struct Entry {
        Beam::SharedBuffer m_buffer;
        boost::posix_time::ptime m_timestamp;
      };
      Beam::local_ptr_t<C> m_channel;
      Beam::local_ptr_t<R> m_time_client;
      Beam::Queue<Entry> m_packets;
      Beam::SharedBuffer m_buffer;
      Beam::OpenState m_open_state;
      Beam::RoutineHandler m_read_loop;

      TmxIpProtocolClient(const TmxIpProtocolClient&) = delete;
      TmxIpProtocolClient& operator =(
        const TmxIpProtocolClient&) = delete;
      void read_loop();
  };

  template<typename CF, typename RF>
  TmxIpProtocolClient(CF&&, RF&&) ->
    TmxIpProtocolClient<std::remove_cvref_t<CF>, std::remove_cvref_t<RF>>;

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  template<Beam::Initializes<C> CF, Beam::Initializes<R> RF>
  TmxIpProtocolClient<C, R>::TmxIpProtocolClient(CF&& channel, RF&& time_client)
    try : m_channel(std::forward<CF>(channel)),
        m_time_client(std::forward<RF>(time_client)) {
      try {
        m_read_loop =
          Beam::spawn(std::bind_front(&TmxIpProtocolClient::read_loop, this));
      } catch(const std::exception&) {
        close();
        throw;
      }
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(Beam::ConnectException(
        "Failed to initialize the TMX IP protocol client."));
    }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  TmxIpProtocolClient<C, R>::~TmxIpProtocolClient() {
    close();
  }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  TmxIpPacket TmxIpProtocolClient<C, R>::read() {
    auto timestamp = boost::posix_time::ptime();
    return read(Beam::out(timestamp));
  }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  TmxIpPacket TmxIpProtocolClient<C, R>::read(
      Beam::Out<boost::posix_time::ptime> timestamp) {
    try {
      reset(m_buffer);
      auto entry = m_packets.pop();
      m_buffer = std::move(entry.m_buffer);
      *timestamp = entry.m_timestamp;
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(
        Beam::IOException("Failed to read TMX IP packet."));
    }
    return TmxIpPacket::parse(
      std::string_view(m_buffer.get_data(), m_buffer.get_size()));
  }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void TmxIpProtocolClient<C, R>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_packets.close();
    m_channel->get_connection().close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void TmxIpProtocolClient<C, R>::read_loop() {
    try {
      while(true) {
        auto buffer = Beam::SharedBuffer();
        m_channel->get_reader().read(Beam::out(buffer));
        m_packets.push(Entry(std::move(buffer), m_time_client->get_time()));
      }
    } catch(const std::exception&) {
      m_packets.close(std::current_exception());
    }
  }
}

#endif
