#ifndef TMX_IP_MESSAGES_HPP
#define TMX_IP_MESSAGES_HPP
#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Stamp/StampFieldReader.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpFields.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpParserException.hpp"

namespace Nexus {

  /** A CDF numeric price or order-price instruction. */
  struct TmxIpPrice {

    /** The interpretation of a price field. */
    enum class Type {

      /** A specified monetary amount. */
      LIMIT,

      /** An order at the market price. */
      MARKET,

      /** An order at the opening price. */
      OPENING,

      /** A must-be-filled order. */
      MUST_BE_FILLED
    };

    /** The price's interpretation. */
    Type m_type;

    /** The monetary amount when the type is LIMIT. */
    Money m_value;

    /** Parses a CDF Price, including its empty-field market-price default. */
    static TmxIpPrice parse(std::string_view source);
  };

  /** CDF timestamps retain the source local time and native clock precision. */
  struct TmxIpMessageHeader {

    /** The STAMP transmission time, which changes on retransmission. */
    boost::posix_time::ptime m_timestamp;

    /** The time of the business action, when supplied. */
    boost::optional<boost::posix_time::ptime> m_trading_timestamp;

    /** The originating venue code, when supplied. */
    boost::optional<std::string_view> m_exchange;

    /** The venue book or book list, when supplied. */
    boost::optional<std::string_view> m_book_type;
  };

  /**
   * A public order in the initial book.
   * Text views share the source buffer lifetime.
   */
  struct TmxIpBookOrder {

    /** CDF symbol (tag 55). */
    std::string_view m_symbol;

    /** CDF order id (tag 40). */
    std::string_view m_order_id;

    /** CDF broker (tag 70). */
    std::uint64_t m_broker;

    /** CDF side (tag 197). */
    Side m_side;

    /** CDF quantity (tag 64). */
    Quantity m_quantity;

    /** CDF public price (tag 196). */
    boost::optional<TmxIpPrice> m_public_price;

    /** CDF price (tag 41). */
    boost::optional<TmxIpPrice> m_price;

    /** CDF priority timestamp (tag 178). */
    boost::optional<boost::posix_time::ptime> m_priority_timestamp;

    /** CDF settlement terms (tag 53). */
    boost::optional<std::string_view> m_settlement_terms;

    /** CDF is nonresident (tag 168). */
    boost::optional<bool> m_is_nonresident;
  };

  /**
   * The public fields for one side of a trade.
   * Text views share the source buffer lifetime.
   */
  struct TmxIpTradeSide {

    /** CDF order id (tag 40). */
    boost::optional<std::string_view> m_order_id;

    /** CDF broker (tag 70). */
    boost::optional<std::uint64_t> m_broker;

    /** CDF display quantity (tag 150). */
    boost::optional<Quantity> m_display_quantity;

    /** CDF priority timestamp (tag 178). */
    boost::optional<boost::posix_time::ptime> m_priority_timestamp;

    /** CDF trade timestamp (tag 264). */
    boost::optional<boost::posix_time::ptime> m_trade_timestamp;
  };

  /**
   * An MBX order key and its optional limit price.
   * Text views share the source buffer lifetime.
   */
  struct TmxIpOrderPrice {

    /** CDF key (tag 192). */
    boost::optional<std::string_view> m_key;

    /** CDF price (tag 41). */
    boost::optional<TmxIpPrice> m_price;
  };

  /**
   * Public trading fields from a CDF SymbolInfo message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpSymbolStatus {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("SymbolInfo");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** The CDF business action. */
    std::string_view m_action;

    /** CDF symbol (tag 55). */
    std::string_view m_symbol;

    /** CDF listing market (tag 554). */
    boost::optional<std::string_view> m_listing_market;

    /** CDF currency (tag 58). */
    boost::optional<std::string_view> m_currency;

    /** CDF board lot (tag 115). */
    boost::optional<std::uint64_t> m_board_lot;

    /** CDF stock group (tag 282). */
    boost::optional<std::uint64_t> m_stock_group;

    /** CDF stock state (tag 161). */
    boost::optional<std::string_view> m_stock_state;

    /** CDF name (tag 177). */
    boost::optional<std::string_view> m_name;

    /** CDF product type (tag 105). */
    boost::optional<std::string_view> m_product_type;

    /** CDF is test symbol (tag 665). */
    boost::optional<bool> m_is_test_symbol;

    /** CDF is last message (tag 113). */
    boost::optional<bool> m_is_last_message;

    /** CDF number of messages (tag 111). */
    boost::optional<std::uint64_t> m_number_of_messages;

    /** CDF total messages (tag 112). */
    boost::optional<std::uint64_t> m_total_messages;

    /** Parses the message and its required trading fields. */
    static TmxIpSymbolStatus parse(const StampMessage& message);
  };

  /**
   * Public trading fields from a CDF OrderInfo message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpOrderBook {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("OrderInfo");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** The CDF business action. */
    std::string_view m_action;

    /** CDF is last message (tag 113). */
    boost::optional<bool> m_is_last_message;

    /** CDF number of messages (tag 111). */
    boost::optional<std::uint64_t> m_number_of_messages;

    /** CDF total messages (tag 112). */
    boost::optional<std::uint64_t> m_total_messages;

    /** The records in source index order. */
    std::vector<TmxIpBookOrder> m_orders;

    /** Parses the message and its required trading fields. */
    static TmxIpOrderBook parse(const StampMessage& message);
  };

  /**
   * Public trading fields from a CDF ClearOrderInfo message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpClearOrderBook {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("ClearOrderInfo");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** The CDF business action. */
    std::string_view m_action;

    /** CDF symbol (tag 55). */
    std::string_view m_symbol;

    /** Parses the message and its required trading fields. */
    static TmxIpClearOrderBook parse(const StampMessage& message);
  };

  /**
   * Public trading fields from a CDF OrderCancelResp message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpOrderCancelReport {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("OrderCancelResp");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** The CDF business action. */
    std::string_view m_action;

    /** CDF symbol (tag 55). */
    std::string_view m_symbol;

    /** CDF confirmation (tag 16). */
    std::string_view m_confirmation;

    /** CDF order id (tag 40). */
    boost::optional<std::string_view> m_order_id;

    /** CDF previous order id (tag 11). */
    boost::optional<std::string_view> m_previous_order_id;

    /** CDF broker (tag 70). */
    boost::optional<std::uint64_t> m_broker;

    /** CDF public price (tag 196). */
    TmxIpPrice m_public_price;

    /** CDF quantity (tag 64). */
    Quantity m_quantity;

    /** CDF priority timestamp (tag 178). */
    boost::optional<boost::posix_time::ptime> m_priority_timestamp;

    /** CDF priority quantity (tag 68). */
    boost::optional<Quantity> m_priority_quantity;

    /** CDF minimum fill quantity (tag 31). */
    boost::optional<Quantity> m_minimum_fill_quantity;

    /** CDF lots of (tag 74). */
    boost::optional<Quantity> m_lots_of;

    /** CDF settlement terms (tag 53). */
    boost::optional<std::string_view> m_settlement_terms;

    /** CDF priority status (tag 639). */
    boost::optional<std::string_view> m_priority_status;

    /** CDF previous price (tag 642). */
    boost::optional<TmxIpPrice> m_previous_price;

    /** CDF is bypass (tag 503). */
    boost::optional<bool> m_is_bypass;

    /** CDF is nonresident (tag 168). */
    boost::optional<bool> m_is_nonresident;

    /** Parses the message and its required trading fields. */
    static TmxIpOrderCancelReport parse(const StampMessage& message);
  };

  /**
   * Public trading fields from a CDF or CLS TradeReport message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpTradeReport {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("TradeReport");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** The CDF business action. */
    std::string_view m_action;

    /** CDF symbol (tag 55). */
    std::string_view m_symbol;

    /** CDF price (tag 41). */
    TmxIpPrice m_price;

    /** CDF quantity (tag 64). */
    Quantity m_quantity;

    /** CDF trade id (tag 220). */
    boost::optional<std::string_view> m_trade_id;

    /** CDF original trade id (tag 506). */
    boost::optional<std::string_view> m_original_trade_id;

    /** CDF previous order id (tag 11). */
    boost::optional<std::string_view> m_previous_order_id;

    /** CDF cross type (tag 390). */
    boost::optional<std::string_view> m_cross_type;

    /** CDF settlement terms (tag 53). */
    boost::optional<std::string_view> m_settlement_terms;

    /** CDF opening auction (tag 574). */
    boost::optional<std::string_view> m_opening_auction;

    /** CDF market state (tag 159). */
    boost::optional<std::string_view> m_market_state;

    /** CDF last sale (tag 114). */
    boost::optional<Money> m_last_sale;

    /** CDF is correction (tag 183). */
    boost::optional<bool> m_is_correction;

    /** CDF is extended hours (tag 76). */
    boost::optional<bool> m_is_extended_hours;

    /** CDF is bypass (tag 503). */
    boost::optional<bool> m_is_bypass;

    /** CDF is nonresident (tag 168). */
    boost::optional<bool> m_is_nonresident;

    /** CDF is dark (tag 617). */
    boost::optional<bool> m_is_dark;

    /** CDF is mid only (tag 684). */
    boost::optional<bool> m_is_mid_only;

    /** CDF is conditional (tag 688). */
    boost::optional<bool> m_is_conditional;

    /** CDF is market on close (tag 494). */
    boost::optional<bool> m_is_market_on_close;

    /** CDF melo (tag 689). */
    boost::optional<std::string_view> m_melo;

    /** CDF purestream (tag 703). */
    boost::optional<std::string_view> m_purestream;

    /** Buy-side fields at index zero and sell-side fields at index one. */
    std::array<TmxIpTradeSide, 2> m_sides;

    /** Parses the message and its required trading fields. */
    static TmxIpTradeReport parse(const StampMessage& message);
  };

  /**
   * Public trading fields from a CDF StockStatus message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpStockStatus {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("StockStatus");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** CDF symbol (tag 55). */
    boost::optional<std::string_view> m_symbol;

    /** CDF stock state (tag 161). */
    boost::optional<std::string_view> m_stock_state;

    /** CDF sub stock state (tag 361). */
    boost::optional<std::string_view> m_sub_stock_state;

    /** CDF stock group (tag 282). */
    boost::optional<std::uint64_t> m_stock_group;

    /** CDF listing market (tag 554). */
    boost::optional<std::string_view> m_listing_market;

    /** CDF listing tier (tag 690). */
    boost::optional<std::string_view> m_listing_tier;

    /** CDF currency (tag 58). */
    boost::optional<std::string_view> m_currency;

    /** CDF comment (tag 173). */
    boost::optional<std::string_view> m_comment;

    /** CDF calculated closing price (tag 491). */
    boost::optional<TmxIpPrice> m_calculated_closing_price;

    /** CDF moc vwap (tag 495). */
    boost::optional<TmxIpPrice> m_moc_vwap;

    /** CDF is moc eligible (tag 496). */
    boost::optional<bool> m_is_moc_eligible;

    /** CDF accepts anonymous (tag 110). */
    boost::optional<bool> m_accepts_anonymous;

    /** CDF accepts undisplayed (tag 605). */
    boost::optional<bool> m_accepts_undisplayed;

    /** CDF settlement terms (tag 53). */
    boost::optional<std::string_view> m_settlement_terms;

    /** CDF is nonresident (tag 168). */
    boost::optional<bool> m_is_nonresident;

    /** Parses the message and its required trading fields. */
    static TmxIpStockStatus parse(const StampMessage& message);
  };

  /**
   * Public trading fields from a CDF MarketStateChange message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpMarketStateChange {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("MarketStateChange");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** CDF market state (tag 159). */
    boost::optional<std::string_view> m_market_state;

    /** CDF stock group (tag 282). */
    boost::optional<std::uint64_t> m_stock_group;

    /** Parses the message and its required trading fields. */
    static TmxIpMarketStateChange parse(const StampMessage& message);
  };

  /**
   * Public trading fields from a CDF MBXMessage message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpMbxMessage {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("MBXMessage");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** The CDF business action. */
    std::string_view m_action;

    /** CDF symbol (tag 55). */
    std::string_view m_symbol;

    /** CDF calculated opening price (tag 191). */
    TmxIpPrice m_calculated_opening_price;

    /** CDF part number (tag 194). */
    boost::optional<std::uint64_t> m_part_number;

    /** CDF total parts (tag 195). */
    boost::optional<std::uint64_t> m_total_parts;

    /** CDF market state (tag 159). */
    boost::optional<std::string_view> m_market_state;

    /** CDF imbalance side (tag 492). */
    boost::optional<std::string_view> m_imbalance_side;

    /** CDF imbalance quantity (tag 493). */
    boost::optional<std::uint64_t> m_imbalance_quantity;

    /** CDF theoretical opening quantity (tag 654). */
    boost::optional<std::uint64_t> m_theoretical_opening_quantity;

    /** CDF paired quantity (tag 698). */
    boost::optional<std::uint64_t> m_paired_quantity;

    /** The records in source index order. */
    std::vector<TmxIpOrderPrice> m_orders;

    /** Parses the message and its required trading fields. */
    static TmxIpMbxMessage parse(const StampMessage& message);
  };

  /**
   * Public trading fields from a CDF OpeningAuction message.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpOpeningAuction {

    /** The CDF business class. */
    static constexpr auto TYPE = std::string_view("OpeningAuction");

    /** The message timestamps and venue. */
    TmxIpMessageHeader m_header;

    /** The CDF business action. */
    std::string_view m_action;

    /** CDF symbol (tag 55). */
    boost::optional<std::string_view> m_symbol;

    /** CDF calculated opening price (tag 191). */
    boost::optional<TmxIpPrice> m_calculated_opening_price;

    /** CDF paired quantity (tag 578). */
    boost::optional<std::uint64_t> m_paired_quantity;

    /** CDF imbalance side (tag 572). */
    boost::optional<Side> m_imbalance_side;

    /** CDF imbalance quantity (tag 573). */
    boost::optional<std::uint64_t> m_imbalance_quantity;

    /** Parses the message and its required trading fields. */
    static TmxIpOpeningAuction parse(const StampMessage& message);
  };

  /** One side of a consolidated CBBO quote. */
  struct TmxIpCbboQuoteSide {

    /** CBBO public price (tag 196). */
    Money m_price;

    /** Aggregate quantity in shares, not board lots (tag 64). */
    std::uint64_t m_quantity;

    /** The time-priority venue attribution, when supplied (tag 247). */
    boost::optional<std::string_view> m_exchange;
  };

  /**
   * A complete two-sided CBBO Quote message, not a sparse update.
   * Standard CBBO uses transport service CB1 and exchange code Q; the caller
   * selects the stream before parsing its STAMP content.
   * Text views remain valid while the source STAMP buffer is unchanged.
   */
  struct TmxIpCbboQuote {

    /** The CBBO business class and action. */
    static constexpr auto TYPE = std::string_view("Quote");

    /** CBBO symbol (tag 55). */
    std::string_view m_symbol;

    /** The originating STAMP address (control tag 54). */
    std::string_view m_source_address;

    /** The destination STAMP address (control tag 17). */
    std::string_view m_destination_address;

    /** The STAMP sequence number, distinct from transport (control tag 50). */
    std::uint64_t m_sequence;

    /** CDF publication time in source local milliseconds (control tag 501). */
    boost::optional<boost::posix_time::ptime> m_publication_timestamp;

    /** CDF receipt time in source local milliseconds (control tag 502). */
    boost::optional<boost::posix_time::ptime> m_receipt_timestamp;

    /** Consolidation inbound time in local milliseconds (control tag 515). */
    boost::optional<boost::posix_time::ptime> m_inbound_timestamp;

    /** Consolidation outbound time in local milliseconds (control tag 514). */
    boost::optional<boost::posix_time::ptime> m_outbound_timestamp;

    /** Required bid at index zero and ask at index one, including zeros. */
    std::array<TmxIpCbboQuoteSide, 2> m_sides;

    /** Parses a complete quote, preserving absent optional fields. */
    static TmxIpCbboQuote parse(const StampMessage& message);
  };

  /** Concept satisfied by visitors accepting a supported TMX IP message. */
  template<typename F>
  concept IsTmxIpVisitor =
    std::invocable<F, TmxIpSymbolStatus> ||
    std::invocable<F, TmxIpOrderBook> ||
    std::invocable<F, TmxIpClearOrderBook> ||
    std::invocable<F, TmxIpOrderCancelReport> ||
    std::invocable<F, TmxIpTradeReport> ||
    std::invocable<F, TmxIpStockStatus> ||
    std::invocable<F, TmxIpMarketStateChange> ||
    std::invocable<F, TmxIpMbxMessage> ||
    std::invocable<F, TmxIpOpeningAuction> ||
    std::invocable<F, TmxIpCbboQuote> ||
    std::invocable<F, const StampMessage&>;

  /** Calls the first matching visitor; unknown classes use the raw fallback. */
  template<IsTmxIpVisitor F, IsTmxIpVisitor... G>
  decltype(auto) visit(const StampMessage& message, F&& f, G&&... g);

  /** Validates supported TMX IP messages; unknown classes are ignored. */
  void validate(const StampMessage& message);


  inline TmxIpPrice TmxIpPrice::parse(std::string_view source) {
    if(source.empty() || source == "MKT") {
      return TmxIpPrice(Type::MARKET, Money::ZERO);
    } else if(source == "OPG") {
      return TmxIpPrice(Type::OPENING, Money::ZERO);
    } else if(source == "MBF") {
      return TmxIpPrice(Type::MUST_BE_FILLED, Money::ZERO);
    }
    auto separator = source.find('.');
    auto whole = source.substr(0, separator);
    auto digits = [] (std::string_view value, std::size_t maximum) {
      return !value.empty() && value.size() <= maximum &&
        std::ranges::all_of(value, [] (auto character) {
          return character >= '0' && character <= '9';
        });
    };
    constexpr auto WHOLE_DIGITS = std::size_t(6);
    constexpr auto FRACTION_DIGITS = std::size_t(6);
    if(separator != std::string_view::npos) {
      while(source.size() > separator + 1 + FRACTION_DIGITS &&
          source.back() == '0') {
        source.remove_suffix(1);
      }
    }
    auto is_valid = digits(whole, WHOLE_DIGITS) &&
      (separator == std::string_view::npos ||
        digits(source.substr(separator + 1), FRACTION_DIGITS));
    if(!is_valid) {
      boost::throw_with_location(TmxIpParserException("Invalid CDF price."));
    }
    return TmxIpPrice(Type::LIMIT, parse_money(source));
  }

namespace TmxIpDetails {
  template<std::size_t N>
  std::uint64_t number(std::string_view source) {
    auto value = std::uint64_t();
    if(source.empty() || source.size() > N || source.front() == '-') {
      boost::throw_with_location(TmxIpParserException("Invalid CDF number."));
    }
    auto [end, error] =
      std::from_chars(source.data(), source.data() + source.size(), value);
    if(error != std::errc() || end != source.data() + source.size()) {
      boost::throw_with_location(TmxIpParserException("Invalid CDF number."));
    }
    return value;
  }

  inline Quantity volume(std::string_view source) {
    constexpr auto DIGITS = std::size_t(20);
    auto is_valid = !source.empty() && source.size() <= DIGITS &&
      std::ranges::all_of(source, [] (auto character) {
        return character >= '0' && character <= '9';
      });
    if(!is_valid) {
      boost::throw_with_location(TmxIpParserException("Invalid CDF volume."));
    }
    auto value = double();
    auto [end, error] =
      std::from_chars(source.data(), source.data() + source.size(), value);
    if(error != std::errc() || end != source.data() + source.size()) {
      boost::throw_with_location(TmxIpParserException("Invalid CDF volume."));
    }
    return Quantity(value);
  }

  template<std::size_t N>
  std::string_view text(std::string_view source) {
    if(source.empty() || source.size() > N) {
      boost::throw_with_location(TmxIpParserException("Invalid CDF text."));
    }
    return source;
  }

  inline Money numeric_price(std::string_view source) {
    auto price = TmxIpPrice::parse(source);
    if(price.m_type != TmxIpPrice::Type::LIMIT) {
      boost::throw_with_location(
        TmxIpParserException("Expected a numeric CDF price."));
    }
    return price.m_value;
  }

  inline bool flag(std::string_view source) {
    if(source == "Y") {
      return true;
    } else if(source == "N") {
      return false;
    }
    boost::throw_with_location(TmxIpParserException("Invalid CDF flag."));
  }

  inline bool default_flag(std::string_view source) {
    if(source.empty()) {
      return false;
    }
    return flag(source);
  }

  inline Side side(std::string_view source) {
    if(source == "Buy") {
      return Side::BID;
    } else if(source == "Sell") {
      return Side::ASK;
    }
    boost::throw_with_location(TmxIpParserException("Invalid CDF side."));
  }

  inline Side opening_side(std::string_view source) {
    if(source == "BuySide") {
      return Side::BID;
    } else if(source == "SellSide") {
      return Side::ASK;
    } else if(source == "NA") {
      return Side::NONE;
    }
    boost::throw_with_location(
      TmxIpParserException("Invalid CDF opening imbalance side."));
  }

  inline boost::posix_time::ptime timestamp(std::string_view source) {
    constexpr auto DATE_TIME_LENGTH = std::size_t(14);
    if(source.size() < DATE_TIME_LENGTH) {
      boost::throw_with_location(
        TmxIpParserException("Invalid CDF timestamp length."));
    }
    auto precision = source.size() - DATE_TIME_LENGTH;
    if(precision != 2 && precision != 3 && precision != 6 && precision != 9) {
      boost::throw_with_location(
        TmxIpParserException("Invalid CDF timestamp length."));
    }
    auto year = number<4>(source.substr(0, 4));
    auto month = number<2>(source.substr(4, 2));
    auto day = number<2>(source.substr(6, 2));
    auto hours = number<2>(source.substr(8, 2));
    auto minutes = number<2>(source.substr(10, 2));
    auto seconds = number<2>(source.substr(12, 2));
    auto fraction = number<9>(source.substr(DATE_TIME_LENGTH));
    if(hours >= 24 || minutes >= 60 || seconds >= 60) {
      boost::throw_with_location(TmxIpParserException("Invalid CDF time."));
    }
    auto divisor = std::uint64_t(1);
    for(auto i = std::size_t(0); i != precision; ++i) {
      divisor *= 10;
    }
    using Duration = boost::posix_time::time_duration;
    auto ticks = fraction * Duration::ticks_per_second() / divisor;
    try {
      return boost::posix_time::ptime(boost::gregorian::date(
        static_cast<unsigned short>(year), static_cast<unsigned short>(month),
        static_cast<unsigned short>(day)), Duration(hours, minutes, seconds,
          static_cast<Duration::fractional_seconds_type>(ticks)));
    } catch(const std::exception&) {
      boost::throw_with_location(TmxIpParserException("Invalid CDF date."));
    }
  }

  inline TmxIpMessageHeader header(
      const StampMessage& message, const StampFieldReader& fields) {
    return TmxIpMessageHeader(
      StampFieldReader(message.m_control_header).read(
        TmxIpFields::TIMESTAMP, timestamp),
      fields.read_optional(TmxIpFields::TRADING_SYS_TIMESTAMP, timestamp),
      fields.read_optional(TmxIpFields::EXCHANGE_ID, text<3>),
      fields.read_optional(TmxIpFields::BOOK_TYPE, text<32>));
  }

  template<typename F, typename T>
  constexpr auto is_void_invocable = [] {
    if constexpr(std::invocable<F, T>) {
      return std::same_as<std::invoke_result_t<F, T>, void>;
    }
    return true;
  }();

  template<IsTmxIpVisitor F, IsTmxIpVisitor... G>
  decltype(auto) visit(std::string_view business_class,
      const StampMessage& message, F&& f, G&&... g) {
    if constexpr(std::invocable<F, TmxIpSymbolStatus>) {
      if(business_class == TmxIpSymbolStatus::TYPE) {
        return std::forward<F>(f)(TmxIpSymbolStatus::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpOrderBook>) {
      if(business_class == TmxIpOrderBook::TYPE) {
        return std::forward<F>(f)(TmxIpOrderBook::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpClearOrderBook>) {
      if(business_class == TmxIpClearOrderBook::TYPE) {
        return std::forward<F>(f)(TmxIpClearOrderBook::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpOrderCancelReport>) {
      if(business_class == TmxIpOrderCancelReport::TYPE) {
        return std::forward<F>(f)(TmxIpOrderCancelReport::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpTradeReport>) {
      if(business_class == TmxIpTradeReport::TYPE) {
        return std::forward<F>(f)(TmxIpTradeReport::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpStockStatus>) {
      if(business_class == TmxIpStockStatus::TYPE) {
        return std::forward<F>(f)(TmxIpStockStatus::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpMarketStateChange>) {
      if(business_class == TmxIpMarketStateChange::TYPE) {
        return std::forward<F>(f)(TmxIpMarketStateChange::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpMbxMessage>) {
      if(business_class == TmxIpMbxMessage::TYPE) {
        return std::forward<F>(f)(TmxIpMbxMessage::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpOpeningAuction>) {
      if(business_class == TmxIpOpeningAuction::TYPE) {
        return std::forward<F>(f)(TmxIpOpeningAuction::parse(message));
      }
    }
    if constexpr(std::invocable<F, TmxIpCbboQuote>) {
      if(business_class == TmxIpCbboQuote::TYPE) {
        return std::forward<F>(f)(TmxIpCbboQuote::parse(message));
      }
    }
    if constexpr(std::invocable<F, const StampMessage&>) {
      return std::forward<F>(f)(message);
    } else if constexpr(sizeof...(G) != 0) {
      return visit(business_class, message, std::forward<G>(g)...);
    } else if constexpr(
        is_void_invocable<F, TmxIpSymbolStatus> &&
        is_void_invocable<F, TmxIpOrderBook> &&
        is_void_invocable<F, TmxIpClearOrderBook> &&
        is_void_invocable<F, TmxIpOrderCancelReport> &&
        is_void_invocable<F, TmxIpTradeReport> &&
        is_void_invocable<F, TmxIpStockStatus> &&
        is_void_invocable<F, TmxIpMarketStateChange> &&
        is_void_invocable<F, TmxIpMbxMessage> &&
        is_void_invocable<F, TmxIpOpeningAuction> &&
        is_void_invocable<F, TmxIpCbboQuote>) {
      return;
    } else {
      boost::throw_with_location(
        TmxIpParserException("Unhandled CDF business class."));
    }
  }
}

  inline TmxIpSymbolStatus TmxIpSymbolStatus::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpSymbolStatus();
    value.m_header = header(message, fields);
    if(!value.m_header.m_trading_timestamp) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF trading timestamp."));
    }
    value.m_action = fields.read(TmxIpFields::BUSINESS_ACTION, text<35>);
    if(value.m_action != "SymbolStatus") {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business action."));
    }
    value.m_symbol = fields.read(TmxIpFields::SYMBOL, text<17>);
    value.m_listing_market =
      fields.read_optional(TmxIpFields::LISTING_MARKET, text<3>);
    value.m_currency = fields.read_optional(TmxIpFields::CURRENCY, text<3>);
    value.m_board_lot =
      fields.read_optional(TmxIpFields::BOARD_LOT, number<10>);
    value.m_stock_group =
      fields.read_optional(TmxIpFields::STOCK_GROUP, number<2>);
    value.m_stock_state =
      fields.read_optional(TmxIpFields::STOCK_STATE, text<64>);
    value.m_name =
      fields.read_optional(TmxIpFields::SYMBOL_FULL_NAME, text<256>);
    value.m_product_type =
      fields.read_optional(TmxIpFields::PRODUCT_TYPE, text<64>);
    value.m_is_test_symbol =
      fields.read_optional(TmxIpFields::TEST_SYMBOL, default_flag);
    value.m_is_last_message =
      fields.read_optional(TmxIpFields::LAST_MESSAGE, flag);
    value.m_number_of_messages =
      fields.read_optional(TmxIpFields::NUMBER_OF_MESSAGES, number<8>);
    value.m_total_messages =
      fields.read_optional(TmxIpFields::TOTAL_NUM_MESSAGES, number<8>);
    return value;
  }

  inline TmxIpOrderBook TmxIpOrderBook::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpOrderBook();
    value.m_header = header(message, fields);
    if(!value.m_header.m_trading_timestamp) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF trading timestamp."));
    }
    value.m_action = fields.read(TmxIpFields::BUSINESS_ACTION, text<35>);
    if(value.m_action != "OrderBook") {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business action."));
    }
    value.m_is_last_message =
      fields.read_optional(TmxIpFields::LAST_MESSAGE, flag);
    value.m_number_of_messages =
      fields.read_optional(TmxIpFields::NUMBER_OF_MESSAGES, number<8>);
    value.m_total_messages =
      fields.read_optional(TmxIpFields::TOTAL_NUM_MESSAGES, number<8>);
    auto count = fields.get_count({
      TmxIpFields::SYMBOL, TmxIpFields::ORDER_NUMBER,
      TmxIpFields::BROKER_NUMBER, TmxIpFields::MARKET_SIDE, TmxIpFields::VOLUME,
      TmxIpFields::PUBLIC_PRICE, TmxIpFields::PRICE,
      TmxIpFields::PRIORITY_TIMESTAMP, TmxIpFields::SETTLEMENT_TERMS,
      TmxIpFields::NON_RESIDENT});
    value.m_orders.reserve(count);
    if(count == 0) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF book order."));
    }
    for(auto i = std::uint16_t(0); i != count; ++i) {
      auto record = TmxIpBookOrder();
      record.m_symbol = fields.read(TmxIpFields::SYMBOL, i, text<17>);
      record.m_order_id = fields.read(TmxIpFields::ORDER_NUMBER, i, text<18>);
      record.m_broker = fields.read(TmxIpFields::BROKER_NUMBER, i, number<3>);
      record.m_side = fields.read(TmxIpFields::MARKET_SIDE, i, side);
      record.m_quantity = fields.read(TmxIpFields::VOLUME, i, volume);
      record.m_public_price =
        fields.read_optional(TmxIpFields::PUBLIC_PRICE, i, TmxIpPrice::parse);
      record.m_price =
        fields.read_optional(TmxIpFields::PRICE, i, TmxIpPrice::parse);
      record.m_priority_timestamp =
        fields.read_optional(TmxIpFields::PRIORITY_TIMESTAMP, i, timestamp);
      record.m_settlement_terms =
        fields.read_optional(TmxIpFields::SETTLEMENT_TERMS, i, text<6>);
      record.m_is_nonresident =
        fields.read_optional(TmxIpFields::NON_RESIDENT, i, flag);
      value.m_orders.push_back(record);
    }
    return value;
  }

  inline TmxIpClearOrderBook TmxIpClearOrderBook::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpClearOrderBook();
    value.m_header = header(message, fields);
    if(!value.m_header.m_trading_timestamp) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF trading timestamp."));
    }
    value.m_action = fields.read(TmxIpFields::BUSINESS_ACTION, text<35>);
    if(value.m_action != "ClearOrderBook") {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business action."));
    }
    value.m_symbol = fields.read(TmxIpFields::SYMBOL, text<17>);
    return value;
  }

  inline TmxIpOrderCancelReport TmxIpOrderCancelReport::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpOrderCancelReport();
    value.m_header = header(message, fields);
    if(!value.m_header.m_trading_timestamp) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF trading timestamp."));
    }
    value.m_action = fields.read(TmxIpFields::BUSINESS_ACTION, text<35>);
    if(value.m_action != "Buy" && value.m_action != "Sell") {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business action."));
    }
    value.m_symbol = fields.read(TmxIpFields::SYMBOL, text<17>);
    value.m_confirmation =
      fields.read(TmxIpFields::CONFIRMATION_TYPE, text<18>);
    value.m_order_id =
      fields.read_optional(TmxIpFields::ORDER_NUMBER, text<18>);
    value.m_previous_order_id =
      fields.read_optional(TmxIpFields::CFOD_ORDER_NUMBER, text<18>);
    value.m_broker =
      fields.read_optional(TmxIpFields::BROKER_NUMBER, number<3>);
    value.m_public_price =
      fields.read(TmxIpFields::PUBLIC_PRICE, TmxIpPrice::parse);
    value.m_quantity = fields.read(TmxIpFields::VOLUME, volume);
    value.m_priority_timestamp =
      fields.read_optional(TmxIpFields::PRIORITY_TIMESTAMP, timestamp);
    value.m_priority_quantity =
      fields.read_optional(TmxIpFields::PRIORITY_VOLUME, volume);
    value.m_minimum_fill_quantity =
      fields.read_optional(TmxIpFields::MINIMUM_FILL_VOLUME, volume);
    value.m_lots_of = fields.read_optional(TmxIpFields::LOTS_OF, volume);
    value.m_settlement_terms =
      fields.read_optional(TmxIpFields::SETTLEMENT_TERMS, text<6>);
    value.m_priority_status =
      fields.read_optional(TmxIpFields::PRIORITY_STATUS, text<32>);
    value.m_previous_price =
      fields.read_optional(TmxIpFields::PREVIOUS_PRICE, TmxIpPrice::parse);
    value.m_is_bypass = fields.read_optional(TmxIpFields::BYPASS, flag);
    value.m_is_nonresident =
      fields.read_optional(TmxIpFields::NON_RESIDENT, flag);
    auto confirmation = value.m_confirmation;
    if(confirmation != "AssignTimePriority" && confirmation != "Booked" &&
        confirmation != "Cancelled" && confirmation != "PriceAssigned" &&
        confirmation != "Killed") {
      boost::throw_with_location(
        TmxIpParserException("Invalid CDF confirmation."));
    }
    return value;
  }

  inline TmxIpTradeReport TmxIpTradeReport::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpTradeReport();
    value.m_header = header(message, fields);
    if(!value.m_header.m_trading_timestamp) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF trading timestamp."));
    }
    value.m_action = fields.read(TmxIpFields::BUSINESS_ACTION, text<35>);
    if(value.m_action != "Trade" &&
        value.m_action != "Cancelled" &&
        value.m_action != "AuctionTradeIndividual") {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business action."));
    }
    value.m_symbol = fields.read(TmxIpFields::SYMBOL, text<17>);
    value.m_price = fields.read(TmxIpFields::PRICE, TmxIpPrice::parse);
    value.m_quantity = fields.read(TmxIpFields::VOLUME, volume);
    value.m_trade_id =
      fields.read_optional(TmxIpFields::TRADE_NUMBER, text<18>);
    value.m_original_trade_id =
      fields.read_optional(TmxIpFields::ORIG_TRADE_ID, text<64>);
    value.m_previous_order_id =
      fields.read_optional(TmxIpFields::CFOD_ORDER_NUMBER, text<18>);
    value.m_cross_type =
      fields.read_optional(TmxIpFields::CROSS_TYPE, text<32>);
    value.m_settlement_terms =
      fields.read_optional(TmxIpFields::SETTLEMENT_TERMS, text<8>);
    value.m_opening_auction =
      fields.read_optional(TmxIpFields::OPENING_AUCTION, text<1>);
    value.m_market_state =
      fields.read_optional(TmxIpFields::MARKET_STATE, text<64>);
    value.m_last_sale =
      fields.read_optional(TmxIpFields::LAST_SALE, numeric_price);
    value.m_is_correction =
      fields.read_optional(TmxIpFields::TRADE_CORRECTION, flag);
    value.m_is_extended_hours =
      fields.read_optional(TmxIpFields::EXTENDED_HOURS, flag);
    value.m_is_bypass = fields.read_optional(TmxIpFields::BYPASS, flag);
    value.m_is_nonresident =
      fields.read_optional(TmxIpFields::NON_RESIDENT, flag);
    value.m_is_dark = fields.read_optional(TmxIpFields::IS_DARK, default_flag);
    value.m_is_mid_only =
      fields.read_optional(TmxIpFields::IS_MID_ONLY, default_flag);
    value.m_is_conditional =
      fields.read_optional(TmxIpFields::CONDITIONAL, flag);
    value.m_is_market_on_close = fields.read_optional(TmxIpFields::MOC, flag);
    value.m_melo = fields.read_optional(TmxIpFields::M_ELO, text<1>);
    value.m_purestream = fields.read_optional(TmxIpFields::PURESTREAM, text<1>);
    auto count = fields.get_count({TmxIpFields::ORDER_NUMBER,
      TmxIpFields::BROKER_NUMBER, TmxIpFields::DISPLAY_VOLUME,
      TmxIpFields::PRIORITY_TIMESTAMP, TmxIpFields::TRADE_TIMESTAMP});
    if(count > value.m_sides.size()) {
      boost::throw_with_location(
        TmxIpParserException("Invalid CDF trade side index."));
    }
    for(auto i = std::uint16_t(0); i != count; ++i) {
      auto& record = value.m_sides[i];
      record.m_order_id =
        fields.read_optional(TmxIpFields::ORDER_NUMBER, i, text<18>);
      record.m_broker =
        fields.read_optional(TmxIpFields::BROKER_NUMBER, i, number<3>);
      record.m_display_quantity =
        fields.read_optional(TmxIpFields::DISPLAY_VOLUME, i, volume);
      record.m_priority_timestamp =
        fields.read_optional(TmxIpFields::PRIORITY_TIMESTAMP, i, timestamp);
      record.m_trade_timestamp =
        fields.read_optional(TmxIpFields::TRADE_TIMESTAMP, i, timestamp);
    }
    return value;
  }

  inline TmxIpStockStatus TmxIpStockStatus::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpStockStatus();
    value.m_header = header(message, fields);
    if(!value.m_header.m_trading_timestamp) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF trading timestamp."));
    }
    value.m_symbol = fields.read_optional(TmxIpFields::SYMBOL, text<17>);
    value.m_stock_state =
      fields.read_optional(TmxIpFields::STOCK_STATE, text<64>);
    value.m_sub_stock_state =
      fields.read_optional(TmxIpFields::SUB_STOCK_STATE, text<64>);
    value.m_stock_group =
      fields.read_optional(TmxIpFields::STOCK_GROUP, number<2>);
    value.m_listing_market =
      fields.read_optional(TmxIpFields::LISTING_MARKET, text<3>);
    value.m_listing_tier =
      fields.read_optional(TmxIpFields::LISTING_TIER, text<16>);
    value.m_currency = fields.read_optional(TmxIpFields::CURRENCY, text<3>);
    value.m_comment = fields.read_optional(TmxIpFields::COMMENT, text<70>);
    value.m_calculated_closing_price = fields.read_optional(
      TmxIpFields::CALCULATED_CLOSING_PRICE, TmxIpPrice::parse);
    value.m_moc_vwap =
      fields.read_optional(TmxIpFields::MOC_VWAP, TmxIpPrice::parse);
    value.m_is_moc_eligible =
      fields.read_optional(TmxIpFields::MOC_ELIGIBLE, flag);
    value.m_accepts_anonymous =
      fields.read_optional(TmxIpFields::ACCEPT_ANONYMOUS, flag);
    value.m_accepts_undisplayed =
      fields.read_optional(TmxIpFields::ACCEPT_UNDISPLAYED, flag);
    value.m_settlement_terms =
      fields.read_optional(TmxIpFields::SETTLEMENT_TERMS, text<6>);
    value.m_is_nonresident =
      fields.read_optional(TmxIpFields::NON_RESIDENT, flag);
    return value;
  }

  inline TmxIpMarketStateChange TmxIpMarketStateChange::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpMarketStateChange();
    value.m_header = header(message, fields);
    if(!value.m_header.m_trading_timestamp) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF trading timestamp."));
    }
    value.m_market_state =
      fields.read_optional(TmxIpFields::MARKET_STATE, text<64>);
    value.m_stock_group =
      fields.read_optional(TmxIpFields::STOCK_GROUP, number<2>);
    return value;
  }

  inline TmxIpMbxMessage TmxIpMbxMessage::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpMbxMessage();
    value.m_header = header(message, fields);
    if(!value.m_header.m_trading_timestamp) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF trading timestamp."));
    }
    value.m_action = fields.read(TmxIpFields::BUSINESS_ACTION, text<35>);
    if(value.m_action != "AssignCOP" && value.m_action != "AssignLimit") {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business action."));
    }
    value.m_symbol = fields.read(TmxIpFields::SYMBOL, text<17>);
    value.m_calculated_opening_price =
      fields.read(TmxIpFields::CALCULATED_OPENING_PRICE, TmxIpPrice::parse);
    value.m_part_number =
      fields.read_optional(TmxIpFields::MBX_PART_NUMBER, number<9>);
    value.m_total_parts =
      fields.read_optional(TmxIpFields::MBX_TOTAL_PARTS, number<9>);
    value.m_market_state =
      fields.read_optional(TmxIpFields::MARKET_STATE, text<64>);
    value.m_imbalance_side =
      fields.read_optional(TmxIpFields::IMBALANCE_SIDE, text<32>);
    value.m_imbalance_quantity =
      fields.read_optional(TmxIpFields::IMBALANCE_VOLUME, number<9>);
    value.m_theoretical_opening_quantity =
      fields.read_optional(TmxIpFields::THEORETICAL_OPENING_VOLUME, number<8>);
    value.m_paired_quantity =
      fields.read_optional(TmxIpFields::PAIRED_VOLUME, number<9>);
    auto count = fields.get_count({TmxIpFields::ORDER_KEY, TmxIpFields::PRICE});
    value.m_orders.reserve(count);
    for(auto i = std::uint16_t(0); i != count; ++i) {
      auto record = TmxIpOrderPrice();
      record.m_key = fields.read_optional(TmxIpFields::ORDER_KEY, i, text<22>);
      record.m_price =
        fields.read_optional(TmxIpFields::PRICE, i, TmxIpPrice::parse);
      if(!record.m_key && !record.m_price) {
        boost::throw_with_location(
          TmxIpParserException("Missing CDF MBX record."));
      }
      value.m_orders.push_back(record);
    }
    return value;
  }

  inline TmxIpOpeningAuction TmxIpOpeningAuction::parse(
      const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business class."));
    }
    auto value = TmxIpOpeningAuction();
    value.m_header = header(message, fields);
    value.m_action = fields.read(TmxIpFields::BUSINESS_ACTION, text<35>);
    if(value.m_action != "PairedVolume" &&
        value.m_action != "OddlotImbalance") {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CDF business action."));
    }
    value.m_symbol = fields.read_optional(TmxIpFields::SYMBOL, text<17>);
    value.m_calculated_opening_price = fields.read_optional(
      TmxIpFields::CALCULATED_OPENING_PRICE, TmxIpPrice::parse);
    value.m_paired_quantity =
      fields.read_optional(TmxIpFields::OPENING_PAIRED_VOLUME, number<9>);
    value.m_imbalance_side =
      fields.read_optional(TmxIpFields::OPENING_IMBALANCE_SIDE, opening_side);
    value.m_imbalance_quantity =
      fields.read_optional(TmxIpFields::OPENING_IMBALANCE_VOLUME, number<9>);
    return value;
  }

  inline TmxIpCbboQuote TmxIpCbboQuote::parse(const StampMessage& message) {
    using namespace TmxIpDetails;
    auto fields = StampFieldReader(message.m_business_content);
    if(fields.read(TmxIpFields::BUSINESS_CLASS, text<35>) != TYPE ||
        fields.read(TmxIpFields::BUSINESS_ACTION, text<35>) != TYPE) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CBBO business class or action."));
    }
    auto value = TmxIpCbboQuote();
    value.m_symbol = fields.read(TmxIpFields::SYMBOL, text<17>);
    if(fields.get_count({TmxIpFields::PUBLIC_PRICE, TmxIpFields::VOLUME,
        TmxIpFields::EXCHANGE_ID}) > value.m_sides.size()) {
      boost::throw_with_location(
        TmxIpParserException("Unexpected CBBO quote side."));
    }
    for(auto i = std::uint16_t(0); i != value.m_sides.size(); ++i) {
      auto& side = value.m_sides[i];
      side.m_price = fields.read(TmxIpFields::PUBLIC_PRICE, i, numeric_price);
      side.m_quantity = fields.read(TmxIpFields::VOLUME, i, number<10>);
      side.m_exchange =
        fields.read_optional(TmxIpFields::EXCHANGE_ID, i, text<3>);
    }
    auto control = StampFieldReader(message.m_control_header);
    auto address = [] (std::string_view source) {
      return source;
    };
    value.m_source_address = control.read(TmxIpFields::SOURCE_ADDRESS, address);
    value.m_destination_address =
      control.read(TmxIpFields::DEST_ADDRESS, address);
    value.m_sequence = control.read(TmxIpFields::SEQUENCE_NUMBER, number<9>);
    auto milliseconds = [] (std::string_view source) {
      if(source.size() != 17) {
        boost::throw_with_location(
          TmxIpParserException("Invalid CBBO timestamp length."));
      }
      return timestamp(source);
    };
    value.m_publication_timestamp =
      control.read_optional(TmxIpFields::CDF_PUB_TIMESTAMP, milliseconds);
    value.m_receipt_timestamp =
      control.read_optional(TmxIpFields::CDF_RCV_TIMESTAMP, milliseconds);
    value.m_inbound_timestamp =
      control.read_optional(TmxIpFields::CDF_INBOUND_TIMESTAMP, milliseconds);
    value.m_outbound_timestamp =
      control.read_optional(TmxIpFields::CDF_OUTBOUND_TIMESTAMP, milliseconds);
    return value;
  }

  template<IsTmxIpVisitor F, IsTmxIpVisitor... G>
  decltype(auto) visit(const StampMessage& message, F&& f, G&&... g) {
    auto field = message.m_business_content.find(TmxIpFields::BUSINESS_CLASS);
    if(!field || field->m_value.empty()) {
      boost::throw_with_location(
        TmxIpParserException("Missing CDF business class."));
    }
    return TmxIpDetails::visit(
      field->m_value, message, std::forward<F>(f), std::forward<G>(g)...);
  }

  inline void validate(const StampMessage& message) {
    visit(message, [] (const auto&) {});
  }
}

#endif
