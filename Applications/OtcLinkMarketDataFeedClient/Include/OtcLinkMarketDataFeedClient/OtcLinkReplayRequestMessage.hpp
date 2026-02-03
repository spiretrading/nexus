#ifndef OTC_LINK_REPLAY_REQUEST_MESSAGE_HPP
#define OTC_LINK_REPLAY_REQUEST_MESSAGE_HPP
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <Beam/IO/Buffer.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/optional/optional.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkChannelId.hpp"

namespace Nexus {

  /** Stores an OTC Link replay request message. */
  struct OtcLinkReplayRequestMessage {

    /** The request type. */
    enum class Type {

      /** Request to fill gaps in sequence. */
      GAP_FILL = 0,

      /** Request for a snapshot. */
      SNAPSHOT = 1
    };

    /** The message sender. */
    std::string m_sender_comp_id;

    /** Unique ID identifying this request. */
    std::string m_appl_req_id;

    /** The request type. */
    Type m_appl_req_type;

    /** The channel ID for the request. */
    OtcLinkChannelId m_channel;

    /** Sequence number of first message in range. */
    boost::optional<std::uint32_t> m_appl_beg_seq_no;

    /** Sequence number of last message in range. */
    boost::optional<std::uint32_t> m_appl_end_seq_no;

    /**
     * Parses an OtcLinkReplayRequestMessage.
     * @param source The message to parse.
     * @return The OtcLinkReplayRequestMessage represented by the <i>source</i>.
     */
    static OtcLinkReplayRequestMessage parse(std::string_view source);

    /**
     * Creates a gap fill request message.
     * @param sender_comp_id The message sender.
     * @param appl_req_id Unique ID identifying this request.
     * @param channel The channel to request.
     * @param appl_beg_seq_no Sequence number of first message in range.
     * @param appl_end_seq_no Sequence number of last message in range.
     * @return The gap fill request message.
     */
    static OtcLinkReplayRequestMessage make_gap_fill_request(
      const std::string& sender_comp_id, const std::string& appl_req_id,
      OtcLinkChannelId channel, std::uint32_t appl_beg_seq_no,
      std::uint32_t appl_end_seq_no);

    /**
     * Creates a snapshot request message.
     * @param sender_comp_id The message sender.
     * @param appl_req_id Unique ID identifying this request.
     * @param channel The channel to request.
     * @return The snapshot request message.
     */
    static OtcLinkReplayRequestMessage make_snapshot_request(
      const std::string& sender_comp_id, const std::string& appl_req_id,
      OtcLinkChannelId channel);
  };

  /**
   * Writes an OtcLinkReplayRequestMessage to a buffer.
   * @param message The message to write.
   * @param buffer The buffer to write to.
   */
  template<Beam::IsBuffer B>
  void write(const OtcLinkReplayRequestMessage& message, Beam::Out<B> buffer) {
    append(*buffer, std::string_view("35=BW\x01" "49="));
    append(*buffer, std::string_view(message.m_sender_comp_id));
    append(*buffer, std::string_view("\x01" "1346="));
    append(*buffer, std::string_view(message.m_appl_req_id));
    append(*buffer, std::string_view("\x01" "1347="));
    append(*buffer, std::string_view(
      std::to_string(static_cast<int>(message.m_appl_req_type))));
    append(*buffer, std::string_view("\x01" "1355="));
    append(*buffer,
      std::string_view(std::to_string(static_cast<int>(message.m_channel))));
    append(*buffer, std::string_view("\x01"));
    if(message.m_appl_beg_seq_no) {
      append(*buffer, std::string_view("1182="));
      append(
        *buffer, std::string_view(std::to_string(*message.m_appl_beg_seq_no)));
      append(*buffer, std::string_view("\x01"));
    }
    if(message.m_appl_end_seq_no) {
      append(*buffer, std::string_view("1183="));
      append(
        *buffer, std::string_view(std::to_string(*message.m_appl_end_seq_no)));
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

  inline OtcLinkReplayRequestMessage OtcLinkReplayRequestMessage::parse(
      std::string_view source) {
    auto message = OtcLinkReplayRequestMessage();
    message.m_appl_req_type = Type::GAP_FILL;
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
      if(tag == "49") {
        message.m_sender_comp_id = std::string(value);
      } else if(tag == "1346") {
        message.m_appl_req_id = std::string(value);
      } else if(tag == "1347") {
        message.m_appl_req_type =
          static_cast<Type>(std::stoi(std::string(value)));
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

  inline OtcLinkReplayRequestMessage
      OtcLinkReplayRequestMessage::make_gap_fill_request(
        const std::string& sender_comp_id, const std::string& appl_req_id,
        OtcLinkChannelId channel, std::uint32_t appl_beg_seq_no,
        std::uint32_t appl_end_seq_no) {
    auto message = OtcLinkReplayRequestMessage();
    message.m_sender_comp_id = sender_comp_id;
    message.m_appl_req_id = appl_req_id;
    message.m_appl_req_type = Type::GAP_FILL;
    message.m_channel = channel;
    message.m_appl_beg_seq_no = appl_beg_seq_no;
    message.m_appl_end_seq_no = appl_end_seq_no;
    return message;
  }

  inline OtcLinkReplayRequestMessage
      OtcLinkReplayRequestMessage::make_snapshot_request(
        const std::string& sender_comp_id, const std::string& appl_req_id,
        OtcLinkChannelId channel) {
    auto message = OtcLinkReplayRequestMessage();
    message.m_sender_comp_id = sender_comp_id;
    message.m_appl_req_id = appl_req_id;
    message.m_appl_req_type = Type::SNAPSHOT;
    message.m_channel = channel;
    return message;
  }
}

#endif
