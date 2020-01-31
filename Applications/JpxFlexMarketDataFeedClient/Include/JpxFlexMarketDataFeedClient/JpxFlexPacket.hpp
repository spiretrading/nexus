#ifndef NEXUS_JPX_FLEX_PACKET_HPP
#define NEXUS_JPX_FLEX_PACKET_HPP
#include <cstdint>
#include <stdexcept>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

namespace Nexus::MarketDataService {

  /** Stores a single packet received from a JPX Flex channel. */
  struct JpxFlexPacket {

    /** The length of a packet header. */
    static constexpr auto HEADER_LENGTH = 42;

    /** Specifies the packet's large message type classification. */
    enum class MessageLargeClassification : std::uint8_t {

      /** Issue realtime message. */
      ISSUE_REALTIME_MESSAGE = 1,

      /** Statistics message. */
      STATISTICS_MESSAGE = 2,

      /** Index message. */
      INDEX_MESSAGE = 3,

      /** Issue basic message. */
      ISSUE_BASIC_MESSAGE = 4,

      /** Control message. */
      CONTROL_MESSAGE = 9
    };

    /** Specifies the packet's medium message type classification. */
    enum class MessageMediumClassification : std::uint8_t {

      /** New. */
      ISSUE_REAL_TIME_MESSAGE_NEW = 0,

      /** Backup. */
      ISSUE_REAL_TIME_MESSAGE_BACKUP = 1,

      /** All-Day. */
      ISSUE_REAL_TIME_MESSAGE_ALL_DAY = 2,

      /** Refreshment. */
      ISSUE_REAL_TIME_MESSAGE_REFRESHMENT = 3,

      /** None. */
      STATISTICS_MESSAGE_NONE = 0,

      /** Index. */
      INDEX_MESSAGE_INDEX = 0,

      /** High-speed index. */
      INDEX_MESSAGE_HIGH_SPEED_INDEX = 5,

      /** Regular. */
      ISSUE_BASIC_MESSAGE_REGULAR = 0,

      /** Special. */
      ISSUE_BASIC_MESSAGE_SPECIAL = 1,

      /** Communication control. */
      CONTROL_MESSAGE_COMMUNICATION_CONTROL = 0,

      /** Issue information control. */
      CONTROL_MESSAGE_ISSUE_INFORMATION_CONTROL = 2,

      /** All-day information control. */
      CONTROL_MESSAGE_ALL_DAY_INFORMATION_CONTROL = 4,

      /** Health check control. */
      CONTROL_MESSAGE_HEALTH_CHECK_CONTROL = 5,

      /** Stock base price information control. */
      CONTROL_MESSAGE_STOCK_BASE_PRICE_INFORMATION_CONTROL = 6,

      /** CB base price information control. */
      CONTROL_MESSAGE_CB_BASE_PRICE_INFORMATION_CONTROL = 7,

      /** Multicast group number information control. */
      CONTROL_MESSAGE_MULTICAST_GROUP_NUMBER_INFORMATION_CONTROL = 8,

      /** Backup information control. */
      CONTROL_MESSAGE_BACKUP_INFORMATION_CONTROL = 9,

      /** Refreshment information control. */
      CONTROL_MESSAGE_REFRESHMENT_INFORMATION_CONTROL = 10,

      /** TCP transmission control. */
      CONTROL_MESSAGE_TCP_TRANSMISSION_CONTROL = 90,
    };

    /** Specifies the packet's large issue classification. */
    enum class IssueLargeClassification : std::uint8_t {

      /** Stock related. */
      STOCK_RELATED = 1,

      /** CB related. */
      CB_RELATED = 2
    };

    /** Specifies the packet's medium issue classification. */
    enum class IssueMediumClassification : std::uint8_t {

      /** First section (domestic stock). */
      FIRST_SECTION_DOMESTIC = 11,

      /** Second section (domestic stock). */
      SECOND_SECTION_DOMESTIC = 12,

      /** First section (foreign stock). */
      FIRST_SECTION_FOREIGN = 13,

      /** Mothers (domestic stock). */
      MOTHERS_DOMESTIC = 14,

      /** Domestic beneficiary certificate of investment trust. */
      DOMESTIC_BENEFICIARY_CERTIFICATE_OF_INVESTMENT_TRUST = 15,

      /** Foreign beneficiary certificate of investment trust. */
      FOREIGN_BENEFICIARY_CERTIFICATE_OF_INVESTMENT_TRUST = 16,

      /** Preferred share. */
      PREFERRED_SHARE = 17,

      /** Mothers (foreign stock). */
      MOTHERS = 18,

      /** Investment certificate. */
      INVESTMENT_CERTIFICATE = 19,

      /** Reserved. */
      STOCK_RESERVED = 20,

      /** Second section (foreign stock). */
      SECOND_SECTION_FOREIGN = 21,

      /** TOKYO PRO market (domestic stock). */
      TOKYO_PRO_MARKET_DOMESTIC = 22,

      /** TOKYO PRO market (foreign stock). */
      TOKYO_PRO_MARKET_FOREIGN = 23,

      /** JASDAQ standard (domestic stock). */
      JASDAQ_STANDARD_DOMESTIC = 24,

      /** JASDAQ standard (foreign stock). */
      JASDAQ_STANDARD_FOREIGN = 25,

      /** JASDAQ growth (domestic stock). */
      JASDAQ_GROWTH_DOMESTIC = 26,

      /** JASDAQ growth (foreign stock). */
      JASDAQ_GROWTH_FOREIGN = 27,

      /** Reserved. */
      STOCK_EXCLUDED_RESERVED = 98,

      /** CB. */
      CB = 11,

      /** Reserved. */
      CB_RESERVED = 12,

      /** SW. */
      SQ = 13,

      /** Exchangeable bond. */
      EXCHANGEABLE_BOND = 14,

      /** Reserved. */
      CB_EXCLUDED_RESERVED = 99,
    };

    /** The size of the packet. */
    std::uint32_t m_size;

    /** The multicast group number. */
    std::uint16_t m_group;

    /** The sequence number. */
    std::uint32_t m_sequenceNumber;

    /** The message's large classification code. */
    MessageLargeClassification m_messageLargeClassification;

    /** The message's medium classification code. */
    MessageMediumClassification m_messageMediumClassification;

    /** The exchange code. */
    MarketCode m_exchange;

    /** The issue's large classification code. */
    IssueLargeClassification m_issueLargeClassification;

    /** The issue's medium classification code. */
    IssueMediumClassification m_issueMediumClassification;

    /** The issue's code. */
    Beam::FixedString<12> m_issueCode;

    /** The packet's payload. */
    const char* m_payload;

    /**
     * Parses a JpxFlexPacket.
     * @param source A pointer to the first byte in the packet.
     * @param size The size of a packet.
     */
    static JpxFlexPacket Parse(const char* data, std::size_t size);
  };

  template<typename T>
  T ParseNumber(Beam::Out<const char*> source,
      Beam::Out<std::size_t> remainingSize, std::size_t size) {
    auto value = T{0};
    while(size-- != 0) {
      value = 10 * value + static_cast<T>(**source - '0');
      ++*source;
    }
    *remainingSize -= size;
    return value;
  }

  inline JpxFlexPacket JpxFlexPacket::Parse(const char* source,
      std::size_t size) {
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Packet too short."));
    }
    auto packet = JpxFlexPacket();
    ++source;
    --size;
    packet.m_size = ParseNumber<std::uint32_t>(Beam::Store(source),
      Beam::Store(size), 6);
    packet.m_group = ParseNumber<std::uint16_t>(Beam::Store(source),
      Beam::Store(size), 3);
    packet.m_sequenceNumber = ParseNumber<std::uint32_t>(Beam::Store(source),
      Beam::Store(size), 8);
    packet.m_messageLargeClassification =
      static_cast<MessageLargeClassification>(ParseNumber<std::uint8_t>(
      Beam::Store(source), Beam::Store(size), 1));
    packet.m_messageMediumClassification =
      static_cast<MessageMediumClassification>(ParseNumber<std::uint8_t>(
      Beam::Store(source), Beam::Store(size), 2));
    auto exchangeCode = ParseNumber<std::uint8_t>(Beam::Store(source),
      Beam::Store(size), 1);
    if(exchangeCode == 1) {
      packet.m_exchange = DefaultMarkets::TSE();
    } else if(exchangeCode == 3) {
      packet.m_exchange = DefaultMarkets::NSE();
    } else if(exchangeCode == 6) {
      packet.m_exchange = DefaultMarkets::XFKA();
    } else if(exchangeCode == 8) {
      packet.m_exchange = DefaultMarkets::SSE();
    }
    source += 2;
    size -= 2;
    packet.m_issueLargeClassification = static_cast<IssueLargeClassification>(
      ParseNumber<std::uint8_t>(Beam::Store(source), Beam::Store(size), 2));
    packet.m_issueMediumClassification = static_cast<IssueMediumClassification>(
      ParseNumber<std::uint8_t>(Beam::Store(source), Beam::Store(size), 2));
    packet.m_issueCode = source;
    source += 13;
    size -= 13;
    packet.m_payload = source;
    return packet;
  }
}

#endif
