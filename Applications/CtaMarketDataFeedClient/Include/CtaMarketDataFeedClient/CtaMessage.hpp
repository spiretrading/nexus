#ifndef NEXUS_CTAMESSAGE_HPP
#define NEXUS_CTAMESSAGE_HPP
#include <cstdint>
#include <stdexcept>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus {
namespace MarketDataService {
  struct CtaBlock;

  /*! \struct CtaMessageHeader
      \brief Stores the header of a single CTA message.
   */
  struct CtaMessageHeader {

    //! The size in bytes of the header.
    static constexpr std::size_t SIZE = 26;

    //! The length of the message including this header.
    std::uint16_t m_length;

    //! The message's category.
    std::uint8_t m_category;

    //! The message's type.
    std::uint8_t m_type;

    //! The market participant that generated the message.
    std::uint8_t m_participantId;

    //! The timestamp when the participant generated the message.
    boost::posix_time::ptime m_timestamp;

    //! The index of the message within the block (starting from 1).
    std::uint8_t m_messageId;

    //! Unused.
    std::uint32_t m_transactionId;

    //! Unused.
    std::int64_t m_participantReference;
  };

  /*! \struct CtaMessage
      \brief Stores a single message from a CTA service.
   */
  struct CtaMessage {

    //! The message header.
    CtaMessageHeader m_header;

    //! The message body.
    const char* m_body;

    //! Parses a CtaMessage.
    /*!
      \param block The CtaBlock that the message belongs to.
      \param data A pointer to the first byte in the packet to parse, this
                  pointer will be modified to point to the end of the message.
      \param size The number of bytes available to parse.
      \return The CtaMessage represented by the <i>buffer</i>.
    */
    static CtaMessage Parse(const CtaBlock& block, Beam::Out<const char*> data,
      std::uint16_t size);
  };

  /*! \struct CtaBlockHeader
      \brief Stores the header of a CTA block.
   */
  struct CtaBlockHeader {

    //! The size in bytes of the header.
    static constexpr std::size_t SIZE = 20;

    //! The block version.
    std::uint8_t m_version;

    //! The size of the entire block, including the header.
    std::uint16_t m_size;

    //! Identifies the source of this transmission.
    std::uint8_t m_feedIndicator;

    //! Indicates whether this block is an original message or a retransmission.
    std::uint8_t m_retransmissionIndicator;

    //! The block's sequence number.
    std::uint32_t m_sequenceNumber;

    //! The number of messages contained in the block.
    std::uint8_t m_messageCount;

    //! The timestamp when this message was transmitted.
    boost::posix_time::ptime m_timestamp;
  };

  /*! \struct CtaBlock
      \brief Stores a single CTA block consisting of zero or more CtaMessages.
   */
  struct CtaBlock {

    //! The block header.
    CtaBlockHeader m_header;

    //! A pointer to the first message in the block.
    const char* m_messages;

    //! Parses a CtaBlock.
    /*!
      \param data A pointer to the first byte in the packet to parse, this
                  pointer will be modified to point to the end of the message.
      \param size The number of bytes available to parse.
      \return The CtaBlock represented by the <i>cursor</i>.
    */
    static CtaBlock Parse(Beam::Out<const char*> data, std::uint16_t size);
  };

  inline CtaMessage CtaMessage::Parse(const CtaBlock& block,
      Beam::Out<const char*> data, std::uint16_t size) {
    if(size < CtaMessageHeader::SIZE) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "Buffer too short for message header."});
    }
    CtaMessage message;
    const char* token = *data;
    message.m_header.m_length = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint16_t*>(token));
    if(size < message.m_header.m_length) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "Buffer too short for message body."});
    }
    token += sizeof(std::uint16_t);
    message.m_header.m_category = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(token));
    token += sizeof(std::uint8_t);
    message.m_header.m_type = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(token));
    token += sizeof(std::uint8_t);
    message.m_header.m_participantId = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(token));
    token += sizeof(std::uint8_t);
    auto epochTimestamp = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(token));
    token += sizeof(std::uint32_t);
    auto nanosecondTimestamp = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(token));
    token += sizeof(std::uint32_t);
    if(epochTimestamp != 0 || nanosecondTimestamp != 0) {
      message.m_header.m_timestamp =
        boost::posix_time::from_time_t(epochTimestamp) +
        boost::posix_time::microseconds(nanosecondTimestamp / 1000);
    } else {
      message.m_header.m_timestamp = block.m_header.m_timestamp;
    }
    message.m_header.m_messageId = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(token));
    token += sizeof(std::uint8_t);
    message.m_header.m_transactionId = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(token));
    token += sizeof(std::uint32_t);
    message.m_header.m_participantReference = Beam::FromBigEndian(
      *reinterpret_cast<const std::int64_t*>(token));
    token += sizeof(std::int64_t);
    message.m_body = token;
    *data += message.m_header.m_length;
    return message;
  }

  inline CtaBlock CtaBlock::Parse(Beam::Out<const char*> data,
      std::uint16_t size) {
    if(size < CtaBlockHeader::SIZE) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "Buffer too short for block header."});
    }
    CtaBlock block;
    const char* token = *data;
    block.m_header.m_version = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(token));
    token += sizeof(std::uint8_t);
    block.m_header.m_size = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint16_t*>(token));
    token += sizeof(std::uint16_t);
    if(size < block.m_header.m_size) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "Buffer too short for block messages."});
    }
    block.m_header.m_feedIndicator = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(token));
    token += sizeof(std::uint8_t);
    block.m_header.m_retransmissionIndicator = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(token));
    token += sizeof(std::uint8_t);
    block.m_header.m_sequenceNumber = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(token));
    token += sizeof(std::uint32_t);
    block.m_header.m_messageCount = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(token));
    token += sizeof(std::uint8_t);
    auto epochTimestamp = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(token));
    token += sizeof(std::uint32_t);
    auto nanosecondTimestamp = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(token));
    token += sizeof(std::uint32_t);
    block.m_header.m_timestamp =
      boost::posix_time::from_time_t(epochTimestamp) +
      boost::posix_time::microseconds(nanosecondTimestamp / 1000);
    token += sizeof(std::uint16_t);
    block.m_messages = token;
    *data += block.m_header.m_size;
    return block;
  }
}
}

#endif
