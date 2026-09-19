#ifndef ASX_TRADE_ITCH_GLIMPSE_CLIENT_HPP
#define ASX_TRADE_ITCH_GLIMPSE_CLIENT_HPP
#include <vector>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchMessages.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpClient.hpp"

namespace Nexus {

  /** Stores a complete Glimpse image and its multicast resume position. */
  struct AsxTradeItchSnapshot {

    /** The first multicast sequence to apply after this snapshot. */
    std::uint64_t m_sequence;

    /** The snapshot's ITCH messages, excluding End of Snapshot. */
    std::vector<Beam::SharedBuffer> m_messages;
  };

  /** Concept satisfied by clients loading a partition's Glimpse image. */
  template<typename T>
  concept IsAsxTradeItchGlimpseClient = requires(T& client) {
    { client.load_snapshot() } -> std::same_as<AsxTradeItchSnapshot>;
    { client.close() } -> std::same_as<void>;
  };

  /**
   * Loads a partition's Glimpse snapshot over SoupBinTCP.
   * @tparam C The channel connected to the Glimpse server.
   * @tparam T The timer used for SoupBinTCP heartbeats and timeouts.
   */
  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class AsxTradeItchGlimpseClient {
    public:

      /** The channel connected to the Glimpse server. */
      using Channel = Beam::dereference_t<C>;

      /** The timer used for SoupBinTCP heartbeats and timeouts. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Logs into Glimpse starting at SoupBinTCP sequence one.
       * @param username The login username.
       * @param password The login password.
       * @param channel The channel connected to the Glimpse server.
       * @param timer A one-second timer for heartbeats and timeouts.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
      AsxTradeItchGlimpseClient(std::string_view username,
        std::string_view password, CF&& channel, TF&& timer);

      /**
       * Loads the snapshot and closes the session. Call once per connection.
       * @return The complete image and the first multicast sequence to apply.
       */
      AsxTradeItchSnapshot load_snapshot();

      /** Closes the session and interrupts a pending load. */
      void close();

    private:
      SoupBinTcpClient<C, T> m_client;
  };

  template<typename CF, typename TF>
  AsxTradeItchGlimpseClient(std::string_view, std::string_view, CF&&, TF&&) ->
    AsxTradeItchGlimpseClient<std::remove_cvref_t<CF>, std::remove_cvref_t<TF>>;

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
  AsxTradeItchGlimpseClient<C, T>::AsxTradeItchGlimpseClient(
      std::string_view username, std::string_view password, CF&& channel,
      TF&& timer)
      : m_client(username, password, std::forward<CF>(channel),
          std::forward<TF>(timer)) {
    if(m_client.get_sequence_number() != 1) {
      boost::throw_with_location(
        Beam::ConnectException("Glimpse must start at sequence one."));
    }
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  AsxTradeItchSnapshot AsxTradeItchGlimpseClient<C, T>::load_snapshot() {
    try {
      auto snapshot = AsxTradeItchSnapshot();
      while(true) {
        auto packet = m_client.read();
        if(packet.m_type == 'H' || packet.m_type == '+') {
          continue;
        }
        if(packet.m_type != 'S') {
          boost::throw_with_location(
            Beam::IOException("Glimpse ended before completing the snapshot."));
        }
        auto message = AsxTradeItchMessage::parse(packet.get_payload());
        if(message.m_type == AsxTradeItchEndOfSnapshot::TYPE) {
          snapshot.m_sequence =
            AsxTradeItchEndOfSnapshot::parse(message).m_sequence;
          close();
          return snapshot;
        }
        validate(message);
        snapshot.m_messages.emplace_back(
          packet.get_payload().data(), packet.get_payload().size());
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchGlimpseClient<C, T>::close() {
    m_client.close();
  }
}

#endif
