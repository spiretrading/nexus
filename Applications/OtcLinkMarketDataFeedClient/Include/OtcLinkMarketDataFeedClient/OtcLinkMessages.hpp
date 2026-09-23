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

  /** Concept satisfied by callables accepting an OTC Link message type. */
  template<typename F>
  concept IsOtcLinkVisitor =
    !std::is_member_pointer_v<std::remove_cvref_t<F>> && (
      std::invocable<F, const OtcLinkMessage&> ||
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
    if constexpr(std::invocable<F, const OtcLinkMessage&>) {
      return std::forward<F>(f)(message);
    } else if constexpr(sizeof...(G) != 0) {
      return visit(message, std::forward<G>(g)...);
    } else if constexpr(OtcLinkDetails::is_void_invocable<F,
        OtcLinkSecurity, OtcLinkFractionalSecurity, OtcLinkMarketOpen,
        OtcLinkMarketClose, OtcLinkSpinStart, OtcLinkSpinEnd>) {
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
