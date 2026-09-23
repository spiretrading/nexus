#ifndef OTC_LINK_PROTOCOL_CLIENT_HPP
#define OTC_LINK_PROTOCOL_CLIENT_HPP
#include <functional>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkPacket.hpp"

namespace Nexus {

  /** Concept satisfied by types delivering the packets of an OTC Link feed. */
  template<typename T>
  concept IsOtcLinkProtocolClient = requires(T& t) {
    { t.read() } -> std::same_as<OtcLinkPacket>;
    { t.read(std::declval<Beam::Out<boost::posix_time::ptime>>()) } ->
      std::same_as<OtcLinkPacket>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Reads the packets delivered by an OTC Link multicast feed.
   * @tparam C The type of Channel delivering one complete datagram per read.
   * @tparam R The type of TimeClient providing receive timestamps.
   */
  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  class OtcLinkProtocolClient {
    public:

      /** The type of channel receiving the feed. */
      using Channel = Beam::dereference_t<C>;

      /** The type of time client providing receive timestamps. */
      using TimeClient = Beam::dereference_t<R>;

      /**
       * Constructs an OtcLinkProtocolClient.
       * @param channel The channel receiving the feed.
       * @param time_client The time client providing receive timestamps.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<R> RF>
      OtcLinkProtocolClient(CF&& channel, RF&& time_client);

      ~OtcLinkProtocolClient();

      /**
       * Reads the next packet from the feed.
       * The returned views are valid until the next read or destruction.
       */
      OtcLinkPacket read();

      /**
       * Reads the next packet and its receive timestamp.
       * The returned views are valid until the next read or destruction.
       * @param timestamp Receives the time the datagram was read.
       */
      OtcLinkPacket read(Beam::Out<boost::posix_time::ptime> timestamp);

      /** Closes the feed connection and releases pending reads. */
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

      OtcLinkProtocolClient(const OtcLinkProtocolClient&) = delete;
      OtcLinkProtocolClient& operator =(const OtcLinkProtocolClient&) = delete;
      void read_loop();
  };

  template<typename CF, typename RF>
  OtcLinkProtocolClient(CF&&, RF&&) ->
    OtcLinkProtocolClient<std::remove_cvref_t<CF>, std::remove_cvref_t<RF>>;

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  template<Beam::Initializes<C> CF, Beam::Initializes<R> RF>
  OtcLinkProtocolClient<C, R>::OtcLinkProtocolClient(
      CF&& channel, RF&& time_client)
    try : m_channel(std::forward<CF>(channel)),
          m_time_client(std::forward<RF>(time_client)) {
      try {
        m_read_loop =
          Beam::spawn(std::bind_front(&OtcLinkProtocolClient::read_loop, this));
      } catch(const std::exception&) {
        close();
        throw;
      }
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(Beam::ConnectException(
        "Failed to initialize the OTC Link protocol client."));
    }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  OtcLinkProtocolClient<C, R>::~OtcLinkProtocolClient() {
    close();
  }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  OtcLinkPacket OtcLinkProtocolClient<C, R>::read() {
    auto timestamp = boost::posix_time::ptime();
    return read(Beam::out(timestamp));
  }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  OtcLinkPacket OtcLinkProtocolClient<C, R>::read(
      Beam::Out<boost::posix_time::ptime> timestamp) {
    try {
      reset(m_buffer);
      auto entry = m_packets.pop();
      m_buffer = std::move(entry.m_buffer);
      *timestamp = entry.m_timestamp;
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(
        Beam::IOException("Failed to read OTC Link packet."));
    }
    return OtcLinkPacket::parse(
      std::string_view(m_buffer.get_data(), m_buffer.get_size()));
  }

  template<typename C, typename R> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void OtcLinkProtocolClient<C, R>::close() {
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
  void OtcLinkProtocolClient<C, R>::read_loop() {
    try {
      while(true) {
        auto buffer = Beam::SharedBuffer();
        m_channel->get_reader().read(Beam::out(buffer));
        auto timestamp = m_time_client->get_time();
        m_packets.push(Entry(std::move(buffer), timestamp));
      }
    } catch(const std::exception&) {
      m_packets.close(std::current_exception());
    }
  }
}

#endif
