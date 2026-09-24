#ifndef OTC_LINK_MESSAGES_HPP
#define OTC_LINK_MESSAGES_HPP
#include <iomanip>
#include <type_traits>
#include <utility>
#include "OtcLinkMarketDataFeedClient/OtcLinkPacket.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkSnapshot.hpp"
#ifdef DELETE
  #undef DELETE
#endif

namespace Nexus {

  /** Identifies a change to security reference data. */
  enum class OtcLinkSecurityAction : std::uint8_t {

    /** Changes an existing security. */
    UPDATE = 1,

    /** Introduces a security. */
    ADD = 2,

    /** Removes a security. */
    DELETE = 3,

    /** Supplies a security in a snapshot. */
    SPIN = 4
  };

  /** Identifies a security's asset class. */
  enum class OtcLinkAssetClass : std::uint8_t {

    /** An equity security. */
    EQUITY = 1,

    /** A fixed-income security. */
    FIXED_INCOME = 2
  };

  /** The market tier assigned by OTC Markets. */
  enum class OtcLinkTier : std::uint8_t {

    /** No assigned tier. */
    NONE = 0,

    /** OTCQX U.S. */
    OTCQX_US = 2,

    /** OTCQX International. */
    OTCQX_INTERNATIONAL = 6,

    /** OTCQB. */
    OTCQB = 10,

    /** OTCID. */
    OTCID = 20,

    /** Pink Limited. */
    PINK_LIMITED = 21,

    /** Grey Market. */
    GREY_MARKET = 30,

    /** Expert Market. */
    EXPERT_MARKET = 40,

    /** OTC Bonds. */
    OTC_BONDS = 50
  };

  /** Security reference data; text views borrow the source message. */
  struct OtcLinkSecurity {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(9);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 32;

    /** Identifies security attributes. */
    enum class Flag : std::uint8_t {

      /** Eligible for proprietary quoting under SEC Rule 15c2-11. */
      PROPRIETARY_QUOTE_ELIGIBLE = 0x01,

      /** Carries a Caveat Emptor warning. */
      CAVEAT_EMPTOR = 0x02,

      /** Restricted to qualified institutional buyers or their agents. */
      QIB_ONLY = 0x04,

      /** May only be quoted unsolicited. */
      UNSOLICITED_ONLY = 0x08,

      /** A multiplier applies to the security's quotes. */
      MULTIPLIER = 0x10,

      /** Eligible for OTC Link ECN trading. */
      ECN_ELIGIBLE = 0x20,

      /** OTC Link messaging is disabled. */
      MESSAGING_DISABLED = 0x40,

      /** Quotes can be excluded from the inside for unresponsiveness. */
      SATURATION_ELIGIBLE = 0x80
    };

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The ticker symbol, possibly empty for fixed-income securities. */
    std::string_view m_symbol;

    /** The last update time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The change to the security definition. */
    OtcLinkSecurityAction m_action;

    /** The security's asset class. */
    OtcLinkAssetClass m_asset_class;

    /** The OTC Markets security identifier. */
    std::uint32_t m_security;

    /** The security attribute bits. */
    std::uint8_t m_flags;

    /** The assigned market tier. */
    OtcLinkTier m_tier;

    /** The issuer's reporting-standard code. */
    char m_reporting_status;

    /** The security's trading-status code. */
    char m_status;

    /** Parses a Security message. */
    static OtcLinkSecurity parse(const OtcLinkMessage& message);

    /** Returns whether a security attribute is set. */
    bool has_flag(Flag flag) const;
  };

  /** Security reference data with fractional-quoting limits. */
  struct OtcLinkFractionalSecurity : OtcLinkSecurity {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(23);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH =
      OtcLinkSecurity::LENGTH + 2 * sizeof(std::uint32_t);

    /** The minimum notional value accepted for quotes and trade messages. */
    std::uint32_t m_minimum_notional_value;

    /** The minimum quote size; zero permits sizes below one share. */
    std::uint32_t m_minimum_quote_size;

    /** Parses a Fractional Security message. */
    static OtcLinkFractionalSecurity parse(const OtcLinkMessage& message);
  };

  /** Announces the opening of the market. */
  struct OtcLinkMarketOpen {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(13);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 20;

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The opening time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The anticipated closing time in milliseconds since the UTC epoch. */
    std::uint64_t m_close;

    /** Parses a Market Open message. */
    static OtcLinkMarketOpen parse(const OtcLinkMessage& message);
  };

  /** Announces the closing of the market. */
  struct OtcLinkMarketClose {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(14);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 16;

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The closing time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The day's message count. */
    std::uint32_t m_count;

    /** Parses a Market Close message. */
    static OtcLinkMarketClose parse(const OtcLinkMessage& message);
  };

  /** Identifies a quote-book entry change. */
  enum class OtcLinkQuoteAction : std::uint8_t {

    /** Introduces a quote. */
    ADD = 2,

    /** Removes a quote. */
    DELETE = 3,

    /** Supplies a quote in a snapshot. */
    SPIN = 4
  };

  /** Attributes shared by full quotes and quote updates. */
  struct OtcLinkQuoteAttributes {

    /** Identifies quote state and pricing attributes. */
    enum class Flag : std::uint8_t {

      /** An update changes the ask; when clear, it changes the bid. */
      UPDATE_ASK = 0x01,

      /** The quote is open. */
      OPEN = 0x02,

      /** The ask is unsolicited. */
      ASK_UNSOLICITED = 0x04,

      /** The ask has an actual price. */
      ASK_PRICED = 0x08,

      /** The ask represents bid wanted interest. */
      ASK_BID_WANTED = 0x10,

      /** The bid is unsolicited. */
      BID_UNSOLICITED = 0x20,

      /** The bid has an actual price. */
      BID_PRICED = 0x40,

      /** The bid represents offer wanted interest. */
      BID_OFFER_WANTED = 0x80
    };

    /** Identifies extended quote attributes. */
    enum class ExtendedFlag : std::uint8_t {

      /** Both sides are excluded from the inside calculation. */
      SATURATED = 0x01,

      /** The bid supports immediate trade-message responses. */
      BID_AUTO_EXECUTION = 0x02,

      /** The ask supports immediate trade-message responses. */
      ASK_AUTO_EXECUTION = 0x04,

      /** An NMS quote is conditional on filling its displayed size. */
      NMS_CONDITIONAL = 0x08,

      /** The participant accepts fractional trade messages. */
      ACCEPTS_FRACTIONAL_TRADES = 0x10
    };

    /** The quote state and pricing bits. */
    std::uint8_t m_flags;

    /** The extended quote attribute bits, including reserved bits. */
    std::uint8_t m_extended_flags;

    /** Returns whether a quote attribute is set. */
    bool has_flag(Flag flag) const;

    /** Returns whether an extended quote attribute is set. */
    bool has_flag(ExtendedFlag flag) const;
  };

  /** A participant quote; its MPID view borrows the source message. */
  struct OtcLinkQuote : OtcLinkQuoteAttributes {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(1);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 63;

    /** The number of price units per currency unit. */
    static constexpr auto PRICE_SCALE = std::uint64_t(1000000);

    /** The number of size units per share. */
    static constexpr auto SIZE_SCALE = std::uint64_t(1);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The identifier of the participant quote. */
    std::uint32_t m_quote;

    /** The change to the quote-book entry. */
    OtcLinkQuoteAction m_action;

    /** The OTC Markets security identifier. */
    std::uint32_t m_security;

    /** The four-character market participant identifier. */
    std::string_view m_mpid;

    /** The ask price in PRICE_SCALE units. */
    std::uint64_t m_ask_price;

    /** The ask size in SIZE_SCALE units. */
    std::uint32_t m_ask_size;

    /** The ask access adjustment code: positive rebate, negative fee. */
    std::int8_t m_ask_adjustment;

    /** The ask time in milliseconds since the UTC epoch. */
    std::uint64_t m_ask_timestamp;

    /** The bid price in PRICE_SCALE units. */
    std::uint64_t m_bid_price;

    /** The bid size in SIZE_SCALE units. */
    std::uint32_t m_bid_size;

    /** The bid access adjustment code: positive rebate, negative fee. */
    std::int8_t m_bid_adjustment;

    /** The bid time in milliseconds since the UTC epoch. */
    std::uint64_t m_bid_timestamp;

    /** The quote owner's reference identifier. */
    std::uint16_t m_reference;

    /** Parses the quote message. */
    static OtcLinkQuote parse(const OtcLinkMessage& message);
  };

  /** A change to one side of a participant quote. */
  struct OtcLinkQuoteUpdate : OtcLinkQuoteAttributes {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(2);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 33;

    /** The number of price units per currency unit. */
    static constexpr auto PRICE_SCALE = std::uint64_t(1000000);

    /** The number of size units per share. */
    static constexpr auto SIZE_SCALE = std::uint64_t(1);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The identifier of the participant quote. */
    std::uint32_t m_quote;

    /** The updated price in PRICE_SCALE units. */
    std::uint64_t m_price;

    /** The updated size in SIZE_SCALE units. */
    std::uint32_t m_size;

    /** The access adjustment code: positive rebate, negative fee. */
    std::int8_t m_adjustment;

    /** The quote time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The quote owner's reference identifier. */
    std::uint16_t m_reference;

    /** Parses the quote message. */
    static OtcLinkQuoteUpdate parse(const OtcLinkMessage& message);
  };

  /**
   * A fractional participant quote; its MPID view borrows the source message.
   */
  struct OtcLinkFractionalQuote : OtcLinkQuoteAttributes {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(19);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 71;

    /** The number of price units per currency unit. */
    static constexpr auto PRICE_SCALE = std::uint64_t(100000000);

    /** The number of size units per share. */
    static constexpr auto SIZE_SCALE = std::uint64_t(100000000);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The identifier of the participant quote. */
    std::uint32_t m_quote;

    /** The change to the quote-book entry. */
    OtcLinkQuoteAction m_action;

    /** The OTC Markets security identifier. */
    std::uint32_t m_security;

    /** The four-character market participant identifier. */
    std::string_view m_mpid;

    /** The ask price in PRICE_SCALE units. */
    std::uint64_t m_ask_price;

    /** The ask size in SIZE_SCALE units. */
    std::uint64_t m_ask_size;

    /** The ask access adjustment code: positive rebate, negative fee. */
    std::int8_t m_ask_adjustment;

    /** The ask time in milliseconds since the UTC epoch. */
    std::uint64_t m_ask_timestamp;

    /** The bid price in PRICE_SCALE units. */
    std::uint64_t m_bid_price;

    /** The bid size in SIZE_SCALE units. */
    std::uint64_t m_bid_size;

    /** The bid access adjustment code: positive rebate, negative fee. */
    std::int8_t m_bid_adjustment;

    /** The bid time in milliseconds since the UTC epoch. */
    std::uint64_t m_bid_timestamp;

    /** The quote owner's reference identifier. */
    std::uint16_t m_reference;

    /** Parses the quote message. */
    static OtcLinkFractionalQuote parse(const OtcLinkMessage& message);
  };

  /** A change to one side of a fractional participant quote. */
  struct OtcLinkFractionalQuoteUpdate : OtcLinkQuoteAttributes {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(20);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 37;

    /** The number of price units per currency unit. */
    static constexpr auto PRICE_SCALE = std::uint64_t(100000000);

    /** The number of size units per share. */
    static constexpr auto SIZE_SCALE = std::uint64_t(100000000);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The identifier of the participant quote. */
    std::uint32_t m_quote;

    /** The updated price in PRICE_SCALE units. */
    std::uint64_t m_price;

    /** The updated size in SIZE_SCALE units. */
    std::uint64_t m_size;

    /** The access adjustment code: positive rebate, negative fee. */
    std::int8_t m_adjustment;

    /** The quote time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The quote owner's reference identifier. */
    std::uint16_t m_reference;

    /** Parses the quote message. */
    static OtcLinkFractionalQuoteUpdate parse(const OtcLinkMessage& message);
  };

  /** Identifies an inside entry change. */
  enum class OtcLinkInsideAction : std::uint8_t {

    /** Introduces an inside entry. */
    ADD = 2,

    /** Removes an inside entry. */
    DELETE = 3,

    /** Supplies an inside entry in a snapshot. */
    SPIN = 4
  };

  /** Attributes shared by inside quotes and inside updates. */
  struct OtcLinkInsideAttributes {

    /** Identifies inside state and pricing attributes. */
    enum class Flag : std::uint8_t {

      /** An update changes the ask; when clear, it changes the bid. */
      UPDATE_ASK = 0x01,

      /** The inside is open. */
      OPEN = 0x02,

      /** The ask has an actual price. */
      ASK_PRICED = 0x08,

      /** The aggregated ask size exceeds two billion shares. */
      ASK_SIZE_OVERFLOW = 0x10,

      /** The bid has an actual price. */
      BID_PRICED = 0x40,

      /** The aggregated bid size exceeds two billion shares. */
      BID_SIZE_OVERFLOW = 0x80
    };

    /** The inside state and pricing bits, including reserved bits. */
    std::uint8_t m_flags;

    /** Returns whether an inside attribute is set. */
    bool has_flag(Flag flag) const;
  };

  /** The best bid and ask aggregated across market participants. */
  struct OtcLinkInside : OtcLinkInsideAttributes {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(3);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 56;

    /** The number of price units per currency unit. */
    static constexpr auto PRICE_SCALE = std::uint64_t(1000000);

    /** The number of size units per share. */
    static constexpr auto SIZE_SCALE = std::uint64_t(1);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The identifier of the inside entry. */
    std::uint32_t m_inside;

    /** The change to the inside entry. */
    OtcLinkInsideAction m_action;

    /** The OTC Markets security identifier. */
    std::uint32_t m_security;

    /** The ask price in PRICE_SCALE units. */
    std::uint64_t m_ask_price;

    /** The aggregated ask size in SIZE_SCALE units. */
    std::uint32_t m_ask_size;

    /** The ask time in milliseconds since the UTC epoch. */
    std::uint64_t m_ask_timestamp;

    /** The bid price in PRICE_SCALE units. */
    std::uint64_t m_bid_price;

    /** The aggregated bid size in SIZE_SCALE units. */
    std::uint32_t m_bid_size;

    /** The bid time in milliseconds since the UTC epoch. */
    std::uint64_t m_bid_timestamp;

    /** The number of participants at the best ask. */
    std::uint8_t m_ask_participants;

    /** The number of participants at the best bid. */
    std::uint8_t m_bid_participants;

    /** Parses the inside message. */
    static OtcLinkInside parse(const OtcLinkMessage& message);
  };

  /** A change to one side of the inside quote. */
  struct OtcLinkInsideUpdate : OtcLinkInsideAttributes {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(4);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 30;

    /** The number of price units per currency unit. */
    static constexpr auto PRICE_SCALE = std::uint64_t(1000000);

    /** The number of size units per share. */
    static constexpr auto SIZE_SCALE = std::uint64_t(1);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The identifier of the inside entry. */
    std::uint32_t m_inside;

    /** The updated price in PRICE_SCALE units. */
    std::uint64_t m_price;

    /** The updated size in SIZE_SCALE units. */
    std::uint32_t m_size;

    /** The inside time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The number of participants at the updated price. */
    std::uint8_t m_participants;

    /** Parses the inside message. */
    static OtcLinkInsideUpdate parse(const OtcLinkMessage& message);
  };

  /** The fractional best bid and ask aggregated across participants. */
  struct OtcLinkFractionalInside : OtcLinkInsideAttributes {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(21);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 64;

    /** The number of price units per currency unit. */
    static constexpr auto PRICE_SCALE = std::uint64_t(100000000);

    /** The number of size units per share. */
    static constexpr auto SIZE_SCALE = std::uint64_t(100000000);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The identifier of the inside entry. */
    std::uint32_t m_inside;

    /** The change to the inside entry. */
    OtcLinkInsideAction m_action;

    /** The OTC Markets security identifier. */
    std::uint32_t m_security;

    /** The ask price in PRICE_SCALE units. */
    std::uint64_t m_ask_price;

    /** The aggregated ask size in SIZE_SCALE units. */
    std::uint64_t m_ask_size;

    /** The ask time in milliseconds since the UTC epoch. */
    std::uint64_t m_ask_timestamp;

    /** The bid price in PRICE_SCALE units. */
    std::uint64_t m_bid_price;

    /** The aggregated bid size in SIZE_SCALE units. */
    std::uint64_t m_bid_size;

    /** The bid time in milliseconds since the UTC epoch. */
    std::uint64_t m_bid_timestamp;

    /** The number of participants at the best ask. */
    std::uint8_t m_ask_participants;

    /** The number of participants at the best bid. */
    std::uint8_t m_bid_participants;

    /** Parses the inside message. */
    static OtcLinkFractionalInside parse(const OtcLinkMessage& message);
  };

  /** A change to one side of the fractional inside quote. */
  struct OtcLinkFractionalInsideUpdate : OtcLinkInsideAttributes {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(22);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 34;

    /** The number of price units per currency unit. */
    static constexpr auto PRICE_SCALE = std::uint64_t(100000000);

    /** The number of size units per share. */
    static constexpr auto SIZE_SCALE = std::uint64_t(100000000);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The identifier of the inside entry. */
    std::uint32_t m_inside;

    /** The updated price in PRICE_SCALE units. */
    std::uint64_t m_price;

    /** The updated size in SIZE_SCALE units. */
    std::uint64_t m_size;

    /** The inside time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The number of participants at the updated price. */
    std::uint8_t m_participants;

    /** Parses the inside message. */
    static OtcLinkFractionalInsideUpdate parse(const OtcLinkMessage& message);
  };

  /** A trade reported on the OTC Link Trade channel. */
  struct OtcLinkTrade {

    /** The wire message type. */
    static constexpr auto TYPE = std::uint8_t(17);

    /** The minimum message length, including its header. */
    static constexpr auto LENGTH = OtcLinkMessage::HEADER_LENGTH + 43;

    /** The number of price units per dollar. */
    static constexpr auto PRICE_SCALE = std::uint64_t(1000000);

    /** Identifies trade status attributes. */
    enum class Status : std::uint8_t {

      /** The trade has non-regular conditions. */
      IRREGULAR = 0x01
    };

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The trade identifier. */
    std::uint32_t m_trade;

    /** The trade flags, currently deprecated or reserved. */
    std::uint8_t m_flags;

    /** The OTC Markets security identifier. */
    std::uint32_t m_security;

    /** The trade status bits. */
    std::uint8_t m_status;

    /** The executing venue; borrows the source message. */
    std::string_view m_venue;

    /** The execution price in PRICE_SCALE units. */
    std::uint64_t m_price;

    /** The number of shares executed. */
    std::uint32_t m_size;

    /** The execution time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** Parses a trade message. */
    static OtcLinkTrade parse(const OtcLinkMessage& message);

    /** Returns whether a trade status attribute is set. */
    bool has_status(Status status) const;
  };

  /** Concept satisfied by callables accepting an OTC Link message type. */
  template<typename F>
  concept IsOtcLinkVisitor =
    !std::is_member_pointer_v<std::remove_cvref_t<F>> && (
      std::invocable<F, const OtcLinkMessage&> ||
      std::invocable<F, OtcLinkTrade> ||
      std::invocable<F, OtcLinkQuote> ||
      std::invocable<F, OtcLinkQuoteUpdate> ||
      std::invocable<F, OtcLinkFractionalQuote> ||
      std::invocable<F, OtcLinkFractionalQuoteUpdate> ||
      std::invocable<F, OtcLinkInside> ||
      std::invocable<F, OtcLinkInsideUpdate> ||
      std::invocable<F, OtcLinkFractionalInside> ||
      std::invocable<F, OtcLinkFractionalInsideUpdate> ||
      std::invocable<F, OtcLinkSecurity> ||
      std::invocable<F, OtcLinkFractionalSecurity> ||
      std::invocable<F, OtcLinkMarketOpen> ||
      std::invocable<F, OtcLinkMarketClose> ||
      std::invocable<F, OtcLinkSpinStart> ||
      std::invocable<F, OtcLinkSpinEnd>);

  /**
   * Passes a parsed message to the first callable able to receive its type.
   * Unknown types are passed as OtcLinkMessage.
   * @param message The message to parse.
   * @param f The callable to try first.
   * @param g The remaining callables.
   * @return The selected callable's return value. Unhandled messages are
   *         ignored for void visitors and throw for value-returning visitors.
   */
  template<IsOtcLinkVisitor F, IsOtcLinkVisitor... G>
  decltype(auto) visit(const OtcLinkMessage& message, F&& f, G&&... g);

  /** Validates the fields of a supported OTC Link message. */
  inline void validate(const OtcLinkMessage& message);

  /** Validates the supported messages in a packet. */
  inline void validate(const OtcLinkPacket& packet);

namespace OtcLinkDetails {
  template<typename F, typename... T>
  constexpr auto is_void_invocable = (requires(F&& f) {
    { std::forward<F>(f)(std::declval<T>()) } -> std::same_as<void>;
  } || ...);

  inline OtcLinkCursor get_cursor(
      const OtcLinkMessage& message, std::uint8_t type) {
    if(message.m_type != type) {
      boost::throw_with_location(
        OtcLinkParserException("Unexpected OTC Link message type."));
    }
    return message.get_cursor();
  }

  inline OtcLinkSecurity parse_security(OtcLinkCursor& cursor) {
    auto security = OtcLinkSecurity();
    security.m_sequence = cursor.read_uint32();
    constexpr auto SYMBOL_LENGTH = 10;
    security.m_symbol = cursor.read_bytes(SYMBOL_LENGTH);
    while(!security.m_symbol.empty() &&
        (security.m_symbol.back() == ' ' || security.m_symbol.back() == '\0')) {
      security.m_symbol.remove_suffix(1);
    }
    security.m_timestamp = cursor.read_uint64();
    auto action = cursor.read_uint8();
    if(action < static_cast<std::uint8_t>(OtcLinkSecurityAction::UPDATE) ||
        action > static_cast<std::uint8_t>(OtcLinkSecurityAction::SPIN)) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link security action."));
    }
    security.m_action = static_cast<OtcLinkSecurityAction>(action);
    security.m_asset_class =
      static_cast<OtcLinkAssetClass>(cursor.read_uint8());
    security.m_security = cursor.read_uint32();
    security.m_flags = cursor.read_uint8();
    security.m_tier = static_cast<OtcLinkTier>(cursor.read_uint8());
    security.m_reporting_status = static_cast<char>(cursor.read_uint8());
    security.m_status = static_cast<char>(cursor.read_uint8());
    return security;
  }

  template<typename T> requires
    std::same_as<T, OtcLinkQuote> || std::same_as<T, OtcLinkFractionalQuote>
  T parse_quote(OtcLinkCursor& cursor) {
    auto quote = T();
    quote.m_sequence = cursor.read_uint32();
    quote.m_quote = cursor.read_uint32();
    auto action = cursor.read_uint8();
    if(action < static_cast<std::uint8_t>(OtcLinkQuoteAction::ADD) ||
        action > static_cast<std::uint8_t>(OtcLinkQuoteAction::SPIN)) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link quote action."));
    }
    quote.m_action = static_cast<OtcLinkQuoteAction>(action);
    quote.m_flags = cursor.read_uint8();
    quote.m_security = cursor.read_uint32();
    constexpr auto MPID_LENGTH = 4;
    quote.m_mpid = cursor.read_bytes(MPID_LENGTH);
    auto read_size = [&] {
      if constexpr(std::same_as<T, OtcLinkQuote>) {
        return cursor.read_uint32();
      } else {
        return cursor.read_uint64();
      }
    };
    quote.m_ask_price = cursor.read_uint64();
    quote.m_ask_size = read_size();
    quote.m_ask_adjustment = static_cast<std::int8_t>(cursor.read_uint8());
    quote.m_ask_timestamp = cursor.read_uint64();
    quote.m_bid_price = cursor.read_uint64();
    quote.m_bid_size = read_size();
    quote.m_bid_adjustment = static_cast<std::int8_t>(cursor.read_uint8());
    quote.m_bid_timestamp = cursor.read_uint64();
    quote.m_reference = cursor.read_uint16();
    quote.m_extended_flags = cursor.read_uint8();
    return quote;
  }

  template<typename T> requires std::same_as<T, OtcLinkQuoteUpdate> ||
    std::same_as<T, OtcLinkFractionalQuoteUpdate>
  T parse_quote_update(OtcLinkCursor& cursor) {
    auto quote = T();
    quote.m_sequence = cursor.read_uint32();
    quote.m_quote = cursor.read_uint32();
    quote.m_flags = cursor.read_uint8();
    quote.m_price = cursor.read_uint64();
    if constexpr(std::same_as<T, OtcLinkQuoteUpdate>) {
      quote.m_size = cursor.read_uint32();
    } else {
      quote.m_size = cursor.read_uint64();
    }
    quote.m_adjustment = static_cast<std::int8_t>(cursor.read_uint8());
    quote.m_timestamp = cursor.read_uint64();
    quote.m_reference = cursor.read_uint16();
    quote.m_extended_flags = cursor.read_uint8();
    return quote;
  }

  template<typename T> requires
    std::same_as<T, OtcLinkInside> || std::same_as<T, OtcLinkFractionalInside>
  T parse_inside(OtcLinkCursor& cursor) {
    auto inside = T();
    inside.m_sequence = cursor.read_uint32();
    inside.m_inside = cursor.read_uint32();
    auto action = cursor.read_uint8();
    if(action < static_cast<std::uint8_t>(OtcLinkInsideAction::ADD) ||
        action > static_cast<std::uint8_t>(OtcLinkInsideAction::SPIN)) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link inside action."));
    }
    inside.m_action = static_cast<OtcLinkInsideAction>(action);
    inside.m_flags = cursor.read_uint8();
    inside.m_security = cursor.read_uint32();
    auto read_size = [&] {
      if constexpr(std::same_as<T, OtcLinkInside>) {
        return cursor.read_uint32();
      } else {
        return cursor.read_uint64();
      }
    };
    inside.m_ask_price = cursor.read_uint64();
    inside.m_ask_size = read_size();
    inside.m_ask_timestamp = cursor.read_uint64();
    inside.m_bid_price = cursor.read_uint64();
    inside.m_bid_size = read_size();
    inside.m_bid_timestamp = cursor.read_uint64();
    inside.m_ask_participants = cursor.read_uint8();
    inside.m_bid_participants = cursor.read_uint8();
    return inside;
  }

  template<typename T> requires std::same_as<T, OtcLinkInsideUpdate> ||
    std::same_as<T, OtcLinkFractionalInsideUpdate>
  T parse_inside_update(OtcLinkCursor& cursor) {
    auto inside = T();
    inside.m_sequence = cursor.read_uint32();
    inside.m_inside = cursor.read_uint32();
    inside.m_flags = cursor.read_uint8();
    inside.m_price = cursor.read_uint64();
    if constexpr(std::same_as<T, OtcLinkInsideUpdate>) {
      inside.m_size = cursor.read_uint32();
    } else {
      inside.m_size = cursor.read_uint64();
    }
    inside.m_timestamp = cursor.read_uint64();
    inside.m_participants = cursor.read_uint8();
    return inside;
  }
}

  inline OtcLinkSecurity OtcLinkSecurity::parse(const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_security(cursor);
  }

  inline bool OtcLinkSecurity::has_flag(Flag flag) const {
    return (m_flags & static_cast<std::uint8_t>(flag)) != 0;
  }

  inline OtcLinkFractionalSecurity OtcLinkFractionalSecurity::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    auto security = OtcLinkFractionalSecurity();
    static_cast<OtcLinkSecurity&>(security) =
      OtcLinkDetails::parse_security(cursor);
    security.m_minimum_notional_value = cursor.read_uint32();
    security.m_minimum_quote_size = cursor.read_uint32();
    return security;
  }

  inline OtcLinkMarketOpen OtcLinkMarketOpen::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    auto result = OtcLinkMarketOpen();
    result.m_sequence = cursor.read_uint32();
    result.m_timestamp = cursor.read_uint64();
    result.m_close = cursor.read_uint64();
    return result;
  }

  inline OtcLinkMarketClose OtcLinkMarketClose::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    auto result = OtcLinkMarketClose();
    result.m_sequence = cursor.read_uint32();
    result.m_timestamp = cursor.read_uint64();
    result.m_count = cursor.read_uint32();
    return result;
  }

  inline bool OtcLinkQuoteAttributes::has_flag(Flag flag) const {
    return (m_flags & static_cast<std::uint8_t>(flag)) != 0;
  }

  inline bool OtcLinkQuoteAttributes::has_flag(ExtendedFlag flag) const {
    return (m_extended_flags & static_cast<std::uint8_t>(flag)) != 0;
  }

  inline OtcLinkQuote OtcLinkQuote::parse(const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_quote<OtcLinkQuote>(cursor);
  }

  inline OtcLinkQuoteUpdate OtcLinkQuoteUpdate::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_quote_update<OtcLinkQuoteUpdate>(cursor);
  }

  inline OtcLinkFractionalQuote OtcLinkFractionalQuote::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_quote<OtcLinkFractionalQuote>(cursor);
  }

  inline OtcLinkFractionalQuoteUpdate OtcLinkFractionalQuoteUpdate::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_quote_update<OtcLinkFractionalQuoteUpdate>(
      cursor);
  }

  inline bool OtcLinkInsideAttributes::has_flag(Flag flag) const {
    return (m_flags & static_cast<std::uint8_t>(flag)) != 0;
  }

  inline OtcLinkInside OtcLinkInside::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_inside<OtcLinkInside>(cursor);
  }

  inline OtcLinkInsideUpdate OtcLinkInsideUpdate::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_inside_update<OtcLinkInsideUpdate>(cursor);
  }

  inline OtcLinkFractionalInside OtcLinkFractionalInside::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_inside<OtcLinkFractionalInside>(cursor);
  }

  inline OtcLinkFractionalInsideUpdate OtcLinkFractionalInsideUpdate::parse(
      const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    return OtcLinkDetails::parse_inside_update<OtcLinkFractionalInsideUpdate>(
      cursor);
  }

  inline OtcLinkTrade OtcLinkTrade::parse(const OtcLinkMessage& message) {
    auto cursor = OtcLinkDetails::get_cursor(message, TYPE);
    auto trade = OtcLinkTrade();
    trade.m_sequence = cursor.read_uint32();
    trade.m_trade = cursor.read_uint32();
    constexpr auto ADD = 2;
    if(cursor.read_uint8() != ADD) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link trade action."));
    }
    trade.m_flags = cursor.read_uint8();
    trade.m_security = cursor.read_uint32();
    trade.m_status = cursor.read_uint8();
    constexpr auto VENUE_LENGTH = 3;
    trade.m_venue = cursor.read_bytes(VENUE_LENGTH);
    constexpr auto DEPRECATED_LENGTH = 5;
    cursor.read_bytes(DEPRECATED_LENGTH);
    trade.m_price = cursor.read_uint64();
    trade.m_size = cursor.read_uint32();
    trade.m_timestamp = cursor.read_uint64();
    return trade;
  }

  inline bool OtcLinkTrade::has_status(Status status) const {
    return (m_status & static_cast<std::uint8_t>(status)) != 0;
  }

  template<IsOtcLinkVisitor F, IsOtcLinkVisitor... G>
  decltype(auto) visit(const OtcLinkMessage& message, F&& f, G&&... g) {
    if constexpr(std::invocable<F, OtcLinkTrade>) {
      if(message.m_type == OtcLinkTrade::TYPE) {
        return std::forward<F>(f)(OtcLinkTrade::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkSecurity>) {
      if(message.m_type == OtcLinkSecurity::TYPE) {
        return std::forward<F>(f)(OtcLinkSecurity::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkFractionalSecurity>) {
      if(message.m_type == OtcLinkFractionalSecurity::TYPE) {
        return std::forward<F>(f)(OtcLinkFractionalSecurity::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkMarketOpen>) {
      if(message.m_type == OtcLinkMarketOpen::TYPE) {
        return std::forward<F>(f)(OtcLinkMarketOpen::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkMarketClose>) {
      if(message.m_type == OtcLinkMarketClose::TYPE) {
        return std::forward<F>(f)(OtcLinkMarketClose::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkSpinStart>) {
      if(message.m_type == OtcLinkSpinStart::TYPE) {
        return std::forward<F>(f)(OtcLinkSpinStart::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkSpinEnd>) {
      if(message.m_type == OtcLinkSpinEnd::TYPE) {
        return std::forward<F>(f)(OtcLinkSpinEnd::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkQuote>) {
      if(message.m_type == OtcLinkQuote::TYPE) {
        return std::forward<F>(f)(OtcLinkQuote::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkQuoteUpdate>) {
      if(message.m_type == OtcLinkQuoteUpdate::TYPE) {
        return std::forward<F>(f)(OtcLinkQuoteUpdate::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkFractionalQuote>) {
      if(message.m_type == OtcLinkFractionalQuote::TYPE) {
        return std::forward<F>(f)(OtcLinkFractionalQuote::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkFractionalQuoteUpdate>) {
      if(message.m_type == OtcLinkFractionalQuoteUpdate::TYPE) {
        return std::forward<F>(f)(OtcLinkFractionalQuoteUpdate::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkInside>) {
      if(message.m_type == OtcLinkInside::TYPE) {
        return std::forward<F>(f)(OtcLinkInside::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkInsideUpdate>) {
      if(message.m_type == OtcLinkInsideUpdate::TYPE) {
        return std::forward<F>(f)(OtcLinkInsideUpdate::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkFractionalInside>) {
      if(message.m_type == OtcLinkFractionalInside::TYPE) {
        return std::forward<F>(f)(OtcLinkFractionalInside::parse(message));
      }
    }
    if constexpr(std::invocable<F, OtcLinkFractionalInsideUpdate>) {
      if(message.m_type == OtcLinkFractionalInsideUpdate::TYPE) {
        return std::forward<F>(f)(
          OtcLinkFractionalInsideUpdate::parse(message));
      }
    }
    if constexpr(std::invocable<F, const OtcLinkMessage&>) {
      return std::forward<F>(f)(message);
    } else if constexpr(sizeof...(G) != 0) {
      return visit(message, std::forward<G>(g)...);
    } else if constexpr(OtcLinkDetails::is_void_invocable<F,
        OtcLinkTrade, OtcLinkSecurity, OtcLinkFractionalSecurity,
        OtcLinkMarketOpen, OtcLinkMarketClose, OtcLinkSpinStart, OtcLinkSpinEnd,
        OtcLinkQuote, OtcLinkQuoteUpdate, OtcLinkFractionalQuote,
        OtcLinkFractionalQuoteUpdate, OtcLinkInside, OtcLinkInsideUpdate,
        OtcLinkFractionalInside, OtcLinkFractionalInsideUpdate>) {
      return;
    } else {
      boost::throw_with_location(
        OtcLinkParserException("Unhandled OTC Link message type."));
    }
  }

  inline void validate(const OtcLinkMessage& message) {
    visit(message, [] (const auto&) {});
  }

  inline void validate(const OtcLinkPacket& packet) {
    for(auto& message : packet) {
      validate(message);
    }
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkSecurity& message) {
    return out << "(security " << message.m_sequence << ' ' <<
      std::quoted(message.m_symbol) << ' ' << message.m_timestamp << ' ' <<
      static_cast<int>(message.m_action) << ' ' <<
      static_cast<int>(message.m_asset_class) << ' ' << message.m_security <<
      ' ' << static_cast<int>(message.m_flags) << ' ' <<
      static_cast<int>(message.m_tier) << ' ' << message.m_reporting_status <<
      ' ' << message.m_status << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkFractionalSecurity& message) {
    return out << "(fractional_security " << message.m_sequence << ' ' <<
      std::quoted(message.m_symbol) << ' ' << message.m_timestamp << ' ' <<
      static_cast<int>(message.m_action) << ' ' <<
      static_cast<int>(message.m_asset_class) << ' ' << message.m_security <<
      ' ' << static_cast<int>(message.m_flags) << ' ' <<
      static_cast<int>(message.m_tier) << ' ' << message.m_reporting_status <<
      ' ' << message.m_status << ' ' << message.m_minimum_notional_value <<
      ' ' << message.m_minimum_quote_size << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkMarketOpen& message) {
    return out << "(market_open " << message.m_sequence << ' ' <<
      message.m_timestamp << ' ' << message.m_close << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkMarketClose& message) {
    return out << "(market_close " << message.m_sequence << ' ' <<
      message.m_timestamp << ' ' << message.m_count << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkQuote& message) {
    return out << "(quote " << static_cast<int>(message.m_flags) << ' ' <<
      static_cast<int>(message.m_extended_flags) << ' ' << message.m_sequence <<
      ' ' << message.m_quote << ' ' << static_cast<int>(message.m_action) <<
      ' ' << message.m_security << ' ' << std::quoted(message.m_mpid) << ' ' <<
      message.m_ask_price << ' ' << message.m_ask_size << ' ' <<
      static_cast<int>(message.m_ask_adjustment) << ' ' <<
      message.m_ask_timestamp << ' ' << message.m_bid_price << ' ' <<
      message.m_bid_size << ' ' << static_cast<int>(message.m_bid_adjustment) <<
      ' ' << message.m_bid_timestamp << ' ' << message.m_reference << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkQuoteUpdate& message) {
    return out << "(quote_update " << static_cast<int>(message.m_flags) <<
      ' ' << static_cast<int>(message.m_extended_flags) << ' ' <<
      message.m_sequence << ' ' << message.m_quote << ' ' << message.m_price <<
      ' ' << message.m_size << ' ' << static_cast<int>(message.m_adjustment) <<
      ' ' << message.m_timestamp << ' ' << message.m_reference << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkFractionalQuote& message) {
    return out << "(fractional_quote " << static_cast<int>(message.m_flags) <<
      ' ' << static_cast<int>(message.m_extended_flags) << ' ' <<
      message.m_sequence << ' ' << message.m_quote << ' ' <<
      static_cast<int>(message.m_action) << ' ' << message.m_security << ' ' <<
      std::quoted(message.m_mpid) << ' ' << message.m_ask_price << ' ' <<
      message.m_ask_size << ' ' << static_cast<int>(message.m_ask_adjustment) <<
      ' ' << message.m_ask_timestamp << ' ' << message.m_bid_price << ' ' <<
      message.m_bid_size << ' ' << static_cast<int>(message.m_bid_adjustment) <<
      ' ' << message.m_bid_timestamp << ' ' << message.m_reference << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkFractionalQuoteUpdate& message) {
    return out << "(fractional_quote_update " <<
      static_cast<int>(message.m_flags) << ' ' <<
      static_cast<int>(message.m_extended_flags) << ' ' << message.m_sequence <<
      ' ' << message.m_quote << ' ' << message.m_price << ' ' <<
      message.m_size << ' ' << static_cast<int>(message.m_adjustment) << ' ' <<
      message.m_timestamp << ' ' << message.m_reference << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkInside& message) {
    return out << "(inside " << static_cast<int>(message.m_flags) << ' ' <<
      message.m_sequence << ' ' << message.m_inside << ' ' <<
      static_cast<int>(message.m_action) << ' ' << message.m_security << ' ' <<
      message.m_ask_price << ' ' << message.m_ask_size << ' ' <<
      message.m_ask_timestamp << ' ' << message.m_bid_price << ' ' <<
      message.m_bid_size << ' ' << message.m_bid_timestamp << ' ' <<
      static_cast<int>(message.m_ask_participants) << ' ' <<
      static_cast<int>(message.m_bid_participants) << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkInsideUpdate& message) {
    return out << "(inside_update " << static_cast<int>(message.m_flags) <<
      ' ' << message.m_sequence << ' ' << message.m_inside << ' ' <<
      message.m_price << ' ' << message.m_size << ' ' << message.m_timestamp <<
      ' ' << static_cast<int>(message.m_participants) << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkFractionalInside& message) {
    return out << "(fractional_inside " << static_cast<int>(message.m_flags) <<
      ' ' << message.m_sequence << ' ' << message.m_inside << ' ' <<
      static_cast<int>(message.m_action) << ' ' << message.m_security << ' ' <<
      message.m_ask_price << ' ' << message.m_ask_size << ' ' <<
      message.m_ask_timestamp << ' ' << message.m_bid_price << ' ' <<
      message.m_bid_size << ' ' << message.m_bid_timestamp << ' ' <<
      static_cast<int>(message.m_ask_participants) << ' ' <<
      static_cast<int>(message.m_bid_participants) << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkFractionalInsideUpdate& message) {
    return out << "(fractional_inside_update " <<
      static_cast<int>(message.m_flags) << ' ' << message.m_sequence << ' ' <<
      message.m_inside << ' ' << message.m_price << ' ' << message.m_size <<
      ' ' << message.m_timestamp << ' ' <<
      static_cast<int>(message.m_participants) << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OtcLinkTrade& message) {
    return out << "(trade " << message.m_sequence << ' ' << message.m_trade <<
      ' ' << static_cast<int>(message.m_flags) << ' ' << message.m_security <<
      ' ' << static_cast<int>(message.m_status) << ' ' <<
      std::quoted(message.m_venue) << ' ' << message.m_price << ' ' <<
      message.m_size << ' ' << message.m_timestamp << ')';
  }
}

#endif
