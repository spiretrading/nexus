#ifndef OTC_LINK_REPLAY_ACK_MESSAGE_HPP
#define OTC_LINK_REPLAY_ACK_MESSAGE_HPP
#include <cstdint>
#include <string>
#include <string_view>
#include <Beam/IO/Buffer.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/optional/optional.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkChannelId.hpp"

namespace Nexus {

  /** Stores an OTC Link replay request acknowledgement message. */
  struct OtcLinkReplayAckMessage {

    /** The response type. */
    enum class ResponseType {

      /** Request successfully processed. */
      SUCCESS = 0,

      /** Request limits exceeded. */
      LIMITS_EXCEEDED = 1,

      /** Messages are not available. */
      MESSAGES_NOT_AVAILABLE = 2,

      /** User not entitled to application. */
      NOT_ENTITLED = 3,

      /** Badly formed request. */
      BAD_REQUEST = 4
    };

    /** The message recipient. */
    std::string m_target_comp_id;

    /** Identifier of the request associated with this ACK message. */
    std::string m_appl_req_id;

    /** The response type. */
    ResponseType m_response_type;

    /** Additional descriptive detail about the response. */
    boost::optional<std::string> m_text;

    /** The channel ID echoed back from the request. */
    OtcLinkChannelId m_channel;

    /** Sequence number of first message in range. */
    boost::optional<std::uint32_t> m_appl_beg_seq_no;

    /** Sequence number of last message in range. */
    boost::optional<std::uint32_t> m_appl_end_seq_no;

    /**
     * Parses an OtcLinkReplayAckMessage.
     * @param source The message to parse.
     * @return The OtcLinkReplayAckMessage represented by the <i>source</i>.
     */
    static OtcLinkReplayAckMessage parse(std::string_view source);

    /**
     * Creates a success acknowledgement message.
     * @param target_comp_id The message recipient.
     * @param appl_req_id Identifier of the request.
     * @param channel The channel ID.
     * @param appl_beg_seq_no Sequence number of first message in range.
     * @param appl_end_seq_no Sequence number of last message in range.
     * @return The success acknowledgement message.
     */
    static OtcLinkReplayAckMessage make_success(
      const std::string& target_comp_id, const std::string& appl_req_id,
      OtcLinkChannelId channel, std::uint32_t appl_beg_seq_no,
      std::uint32_t appl_end_seq_no);

    /**
     * Creates an error acknowledgement message.
     * @param target_comp_id The message recipient.
     * @param appl_req_id Identifier of the request.
     * @param channel The channel ID.
     * @param response_type The error response type.
     * @param text Optional descriptive detail about the error.
     * @return The error acknowledgement message.
     */
    static OtcLinkReplayAckMessage make_error(const std::string& target_comp_id,
      const std::string& appl_req_id, OtcLinkChannelId channel,
      ResponseType response_type,
      const boost::optional<std::string>& text = boost::none);
  };

  /**
   * Writes an OtcLinkReplayAckMessage to a buffer.
   * @param message The message to write.
   * @param buffer The buffer to write to.
   */
  template<Beam::IsBuffer B>
  void write(const OtcLinkReplayAckMessage& message, Beam::Out<B> buffer) {
    append(*buffer, std::string_view("35=BX\x01" "59="));
    append(*buffer, std::string_view(message.m_target_comp_id));
    append(*buffer, std::string_view("\x01" "1346="));
    append(*buffer, std::string_view(message.m_appl_req_id));
    append(*buffer, std::string_view("\x01" "1348="));
    append(*buffer, std::string_view(
      std::to_string(static_cast<int>(message.m_response_type))));
    append(*buffer, std::string_view("\x01"));
    if(message.m_text) {
      append(*buffer, std::string_view("58="));
      append(*buffer, std::string_view(message.m_text.value()));
      append(*buffer, std::string_view("\x01"));
    }
    append(*buffer, std::string_view("1355="));
    append(*buffer,
      std::string_view(std::to_string(static_cast<int>(message.m_channel))));
    append(*buffer, std::string_view("\x01"));
    if(message.m_appl_beg_seq_no) {
      append(*buffer, std::string_view("1182="));
      append(*buffer,
        std::string_view(std::to_string(message.m_appl_beg_seq_no.value())));
      append(*buffer, std::string_view("\x01"));
    }
    if(message.m_appl_end_seq_no) {
      append(*buffer, std::string_view("1183="));
      append(*buffer,
        std::string_view(std::to_string(message.m_appl_end_seq_no.value())));
      append(*buffer, std::string_view("\x01"));
    }
    auto checksum = 0;
    for(auto i = buffer->get_data();
        i != buffer->get_data() + buffer->get_size(); ++i) {
      checksum += static_cast<unsigned char>(*i);
    }
    checksum = checksum % 256;
    auto checksum_str = std::to_string(checksum);
    while(checksum_str.size() < 3) {
      checksum_str = "0" + checksum_str;
    }
    append(*buffer, std::string_view("10="));
    append(*buffer, std::string_view(checksum_str));
    append(*buffer, std::string_view("\x01"));
  }

  inline OtcLinkReplayAckMessage OtcLinkReplayAckMessage::parse(
      std::string_view source) {
    auto message = OtcLinkReplayAckMessage();
    auto position = std::size_t(0);
    while(position < source.size()) {
      auto equals_position = source.find('=', position);
      if(equals_position == std::string_view::npos) {
        break;
      }
      auto tag = source.substr(position, equals_position - position);
      auto soh_position = source.find('\x01', equals_position + 1);
      if(soh_position == std::string_view::npos) {
        soh_position = source.size();
      }
      auto value =
        source.substr(equals_position + 1, soh_position - equals_position - 1);
      if(tag == "59") {
        message.m_target_comp_id = std::string(value);
      } else if(tag == "1346") {
        message.m_appl_req_id = std::string(value);
      } else if(tag == "1348") {
        message.m_response_type =
          static_cast<ResponseType>(std::stoi(std::string(value)));
      } else if(tag == "58") {
        message.m_text = std::string(value);
      } else if(tag == "1355") {
        message.m_channel =
          static_cast<OtcLinkChannelId>(std::stoi(std::string(value)));
      } else if(tag == "1182") {
        message.m_appl_beg_seq_no = std::stoul(std::string(value));
      } else if(tag == "1183") {
        message.m_appl_end_seq_no = std::stoul(std::string(value));
      }
      position = soh_position + 1;
    }
    return message;
  }

  inline OtcLinkReplayAckMessage OtcLinkReplayAckMessage::make_success(
      const std::string& target_comp_id, const std::string& appl_req_id,
      OtcLinkChannelId channel, std::uint32_t appl_beg_seq_no,
      std::uint32_t appl_end_seq_no) {
    auto message = OtcLinkReplayAckMessage();
    message.m_target_comp_id = target_comp_id;
    message.m_appl_req_id = appl_req_id;
    message.m_response_type = ResponseType::SUCCESS;
    message.m_channel = channel;
    message.m_appl_beg_seq_no = appl_beg_seq_no;
    message.m_appl_end_seq_no = appl_end_seq_no;
    return message;
  }

  inline OtcLinkReplayAckMessage OtcLinkReplayAckMessage::make_error(
      const std::string& target_comp_id, const std::string& appl_req_id,
      OtcLinkChannelId channel, ResponseType response_type,
      const boost::optional<std::string>& text) {
    auto message = OtcLinkReplayAckMessage();
    message.m_target_comp_id = target_comp_id;
    message.m_appl_req_id = appl_req_id;
    message.m_response_type = response_type;
    message.m_text = text;
    message.m_channel = channel;
    return message;
  }
}

#endif
