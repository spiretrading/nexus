#ifndef OTC_LINK_MESSAGES_HPP
#define OTC_LINK_MESSAGES_HPP
#include <type_traits>
#include <utility>
#include "OtcLinkMarketDataFeedClient/OtcLinkPacket.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkSnapshot.hpp"

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

  /** Concept satisfied by callables accepting an OTC Link message type. */
  template<typename F>
  concept IsOtcLinkVisitor =
    !std::is_member_pointer_v<std::remove_cvref_t<F>> && (
      std::invocable<F, const OtcLinkMessage&> ||
      std::invocable<F, OtcLinkQuote> ||
      std::invocable<F, OtcLinkQuoteUpdate> ||
      std::invocable<F, OtcLinkFractionalQuote> ||
      std::invocable<F, OtcLinkFractionalQuoteUpdate> ||
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

  template<IsOtcLinkVisitor F, IsOtcLinkVisitor... G>
  decltype(auto) visit(const OtcLinkMessage& message, F&& f, G&&... g) {
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
    if constexpr(std::invocable<F, const OtcLinkMessage&>) {
      return std::forward<F>(f)(message);
    } else if constexpr(sizeof...(G) != 0) {
      return visit(message, std::forward<G>(g)...);
    } else if constexpr(OtcLinkDetails::is_void_invocable<F,
        OtcLinkSecurity, OtcLinkFractionalSecurity, OtcLinkMarketOpen,
        OtcLinkMarketClose, OtcLinkSpinStart, OtcLinkSpinEnd, OtcLinkQuote,
        OtcLinkQuoteUpdate, OtcLinkFractionalQuote,
        OtcLinkFractionalQuoteUpdate>) {
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
}

#endif
