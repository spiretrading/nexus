#ifndef OTC_LINK_RECOVERY_CLIENT_HPP
#define OTC_LINK_RECOVERY_CLIENT_HPP
#include <functional>
#include <random>
#include <type_traits>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkChannelId.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkReplayAckMessage.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkReplayRequestMessage.hpp"

namespace Nexus {

  /** Stores the result of a recovery request. */
  struct OtcLinkRecoveryResult {

    /** The response type. */
    OtcLinkReplayAckMessage::ResponseType m_response;

    /** Optional descriptive detail about the response. */
    std::string m_text;
  };

  /**
   * Implements a client for recovering OTC Link market data.
   * @tparam C The type of channel used to communicate with the server.
   */
  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  class OtcLinkRecoveryClient {
    public:

      /** The type of Channel used to communicate with the recovery server. */
      using Channel = Beam::dereference_t<C>;

      /** Type of function used to create channels to the recovery server. */
      using ChannelBuilder = std::function<C ()>;

      /**
       * Constructs an OtcLinkRecoveryClient.
       * @param sender_comp_id The sender comp ID to use in requests.
       * @param channel_builder Builds channels to the recovery server.
       */
      OtcLinkRecoveryClient(
        std::string sender_comp_id, ChannelBuilder channel_builder);

      /**
       * Requests a snapshot from the recovery server.
       * @param channel The channel to request the snapshot on.
       * @return The result of the request.
       */
      OtcLinkRecoveryResult request_snapshot(OtcLinkChannelId channel);

    private:
      std::string m_sender_comp_id;
      ChannelBuilder m_channel_builder;

      static std::string generate_request_id();
  };

  template<typename S, typename F>
  OtcLinkRecoveryClient(S&&, F) ->
    OtcLinkRecoveryClient<std::invoke_result_t<F>>;

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  OtcLinkRecoveryClient<C>::OtcLinkRecoveryClient(
    std::string sender_comp_id, ChannelBuilder channel_builder)
    : m_sender_comp_id(std::move(sender_comp_id)),
      m_channel_builder(std::move(channel_builder)) {}

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  OtcLinkRecoveryResult OtcLinkRecoveryClient<C>::request_snapshot(
      OtcLinkChannelId channel) {
    auto connection = m_channel_builder();
    auto request_id = generate_request_id();
    auto message = OtcLinkReplayRequestMessage::make_snapshot_request(
      m_sender_comp_id, request_id, channel);
    auto buffer = Beam::SharedBuffer();
    write(message, out(buffer));
    try {
      connection->get_writer().write(buffer);
    } catch(const std::exception& e) {
      return OtcLinkRecoveryResult(
        OtcLinkReplayAckMessage::ResponseType::BAD_REQUEST, e.what());
    }
    reset(buffer);
    try {
      connection->get_reader().read(out(buffer));
    } catch(const Beam::EndOfFileException&) {
      return OtcLinkRecoveryResult(
        OtcLinkReplayAckMessage::ResponseType::MESSAGES_NOT_AVAILABLE,
        "Connection closed.");
    }
    auto ack = OtcLinkReplayAckMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    auto result = OtcLinkRecoveryResult();
    result.m_response = ack.m_response_type;
    if(ack.m_text) {
      result.m_text = *ack.m_text;
    }
    return result;
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  std::string OtcLinkRecoveryClient<C>::generate_request_id() {
    static const auto CHARS =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static const auto CHARS_LENGTH = std::strlen(CHARS);
    static auto generator = std::mt19937(std::random_device()());
    static auto distribution =
      std::uniform_int_distribution<std::size_t>(0, CHARS_LENGTH - 1);
    auto result = std::string(8, '\0');
    for(auto& c : result) {
      c = CHARS[distribution(generator)];
    }
    return result;
  }
}

#endif
