#ifndef TMX_IP_MARKET_DATA_FEED_CLIENT_HPP
#define TMX_IP_MARKET_DATA_FEED_CLIENT_HPP
#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/Queries/TickerAccessor.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpClient.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpConfiguration.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpMessages.hpp"

namespace Nexus {

  /**
   * Publishes market data originating from the TMX IP market data feeds.
   * @tparam C The client delivering ordered STAMP messages.
   * @tparam D The client resolving ticker information.
   * @tparam T The client providing current UTC time.
   * @tparam M The client publishing market data.
   */
  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  class TmxIpMarketDataFeedClient {
    public:

      /** The client delivering ordered STAMP messages. */
      using TmxIpClient = Beam::dereference_t<C>;

      /** The client resolving ticker information. */
      using MarketDataClient = Beam::dereference_t<D>;

      /** The client providing current UTC time. */
      using TimeClient = Beam::dereference_t<T>;

      /** The client publishing market data. */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /**
       * Constructs a TMX IP market data feed client.
       * @param config The feed's configuration.
       * @param schedule The listing venues' trading schedule.
       * @param tmx_ip_client Initializes the ordered message source.
       * @param market_data_client Initializes the ticker information source.
       * @param time_client Initializes the UTC clock.
       * @param feed_client Initializes the market data publisher.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<D> DF,
        Beam::Initializes<T> TF, Beam::Initializes<M> MF>
      TmxIpMarketDataFeedClient(TmxIpConfiguration config,
        TradingSchedule schedule, CF&& tmx_ip_client, DF&& market_data_client,
        TF&& time_client, MF&& feed_client);

      ~TmxIpMarketDataFeedClient();

      /** Returns whether message reception has finished. */
      bool is_finished() const;

      /** Returns the exception that stopped message reception, if any. */
      std::exception_ptr get_exception() const;

      /** Stops message reception and closes the source client. */
      void close();

    private:
      struct OrderEntry {
        Side m_side;
        TmxIpPrice m_price;
        Quantity m_quantity;
        bool m_is_market;
        boost::optional<std::uint64_t> m_broker;
      };
      struct Book {
        Ticker m_ticker;
        Venue m_venue;
        std::string m_mpid;
        std::string m_prefix;
        Quantity m_board_lot;
        std::unordered_map<std::string, OrderEntry> m_orders;
      };
      struct OpeningQuote {
        boost::gregorian::date m_date;
        boost::optional<Money> m_price;
        Quantity m_paired_quantity;
        Side m_imbalance_side;
        Quantity m_imbalance_quantity;
      };
      struct Session {
        boost::gregorian::date m_date;
        boost::optional<boost::posix_time::ptime> m_opening;
        boost::optional<boost::posix_time::ptime> m_closing;
      };
      TmxIpConfiguration m_config;
      TradingSchedule m_schedule;
      Beam::local_ptr_t<C> m_tmx_ip_client;
      Beam::local_ptr_t<D> m_market_data_client;
      Beam::local_ptr_t<T> m_time_client;
      Beam::local_ptr_t<M> m_feed_client;
      std::unordered_map<std::string, boost::optional<TickerInfo>> m_tickers;
      std::unordered_map<Ticker, OpeningQuote> m_opening_quotes;
      std::unordered_map<Venue, Session> m_sessions;
      std::unordered_map<Ticker, Book> m_books;
      boost::gregorian::date m_auction_date;
      std::unordered_set<std::string> m_auction_trades;
      Beam::Sync<std::exception_ptr> m_exception;
      std::atomic_bool m_is_finished;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      static void log(const StampMessage& message);
      static TimeAndSale::Condition get_condition(
        const TmxIpTradeReport& message);
      static std::string get_order_key(const Book& book, Side side,
        boost::optional<std::uint64_t> broker, std::string_view id);
      static Quantity get_quantity(const Book& book, const OrderEntry& order);
      TmxIpMarketDataFeedClient(const TmxIpMarketDataFeedClient&) = delete;

      TmxIpMarketDataFeedClient& operator =(
        const TmxIpMarketDataFeedClient&) = delete;
      Venue get_venue(
        const Ticker& ticker, const TmxIpMessageHeader& header) const;
      Book* find_book(
        std::string_view symbol, const TmxIpMessageHeader& header);
      std::string get_mpid(
        const Book& book, boost::optional<std::uint64_t> broker) const;
      std::string get_broker_name(boost::optional<std::uint64_t> broker) const;
      void update(Book& book, const std::string& key, OrderEntry order,
        boost::posix_time::ptime timestamp);
      void remove(Book& book, const std::string& key,
        boost::posix_time::ptime timestamp);
      void update(const TickerInfo& info, boost::posix_time::ptime timestamp);
      void publish(const TmxIpSymbolStatus& message);
      void publish(const TmxIpOrderBook& message);
      void publish(const TmxIpOrderCancelReport& message);
      void publish(const TmxIpClearOrderBook& message);
      void publish(const TmxIpTradeReport& message);
      void publish_trade(const TmxIpTradeReport& message);
      void update_orders(const TmxIpMbxMessage& message);
      void load_tickers();
      const TickerInfo* find_ticker(std::string_view symbol);
      bool is_eligible(Venue venue, bool is_opening,
        boost::posix_time::ptime timestamp);
      OpeningQuote* find_opening_quote(
        std::string_view symbol, const TmxIpMessageHeader& header);
      void publish(const TmxIpMbxMessage& message);
      void publish(const TmxIpOpeningAuction& message);
      void publish(const TmxIpCbboQuote& message);
      void publish_opening_quote(
        std::string_view symbol, const TmxIpMessageHeader& header);
      void log_error(
        const StampMessage& message, const std::exception& error);
      void read_loop();
  };

  template<typename CF, typename DF, typename TF, typename MF>
  TmxIpMarketDataFeedClient(TmxIpConfiguration, TradingSchedule, CF&&, DF&&,
    TF&&, MF&&) -> TmxIpMarketDataFeedClient<
      std::remove_cvref_t<CF>, std::remove_cvref_t<DF>, std::remove_cvref_t<TF>,
      std::remove_cvref_t<MF>>;

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  template<Beam::Initializes<C> CF, Beam::Initializes<D> DF,
    Beam::Initializes<T> TF, Beam::Initializes<M> MF>
  TmxIpMarketDataFeedClient<C, D, T, M>::TmxIpMarketDataFeedClient(
      TmxIpConfiguration config, TradingSchedule schedule, CF&& tmx_ip_client,
      DF&& market_data_client, TF&& time_client, MF&& feed_client)
      : m_config(std::move(config)),
        m_schedule(std::move(schedule)),
        m_tmx_ip_client(std::forward<CF>(tmx_ip_client)),
        m_market_data_client(std::forward<DF>(market_data_client)),
        m_time_client(std::forward<TF>(time_client)),
        m_feed_client(std::forward<MF>(feed_client)),
        m_is_finished(false) {
    m_read_loop = Beam::spawn([&] { read_loop(); });
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  TmxIpMarketDataFeedClient<C, D, T, M>::~TmxIpMarketDataFeedClient() {
    close();
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  bool TmxIpMarketDataFeedClient<C, D, T, M>::is_finished() const {
    return m_is_finished;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  std::exception_ptr
      TmxIpMarketDataFeedClient<C, D, T, M>::get_exception() const {
    return m_exception.load();
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tmx_ip_client->close();
    m_market_data_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_time_client->close();
    m_open_state.close();
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::log(const StampMessage& message) {
    auto out = std::stringstream();
    auto write_section = [&] (
        std::string_view name, const StampMessage::Section& section) {
      out << " (" << name;
      for(auto& field : section) {
        out << " (" << field.m_identifier << ' ' << field.m_index << ' ' <<
          std::quoted(field.m_value) << ')';
      }
      out << ')';
    };
    out << "(stamp";
    write_section("control", message.m_control_header);
    write_section("business", message.m_business_content);
    out << ")\n";
    std::cout << out.str() << std::flush;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  TimeAndSale::Condition TmxIpMarketDataFeedClient<C, D, T, M>::get_condition(
      const TmxIpTradeReport& message) {
    auto condition = TimeAndSale::Condition();
    auto append = [&] (std::string_view code) {
      if(!condition.m_code.empty()) {
        condition.m_code += ';';
      }
      condition.m_code += code;
    };
    auto append_mapped = [&] (
        std::string_view value, const auto& codes, std::string_view fallback) {
      for(auto& [name, code] : codes) {
        if(value == name) {
          append(code);
          return;
        }
      }
      append(fallback);
    };
    if(message.m_action == "Cancelled") {
      condition.m_type = TimeAndSale::Condition::Type::CANCELLATION;
      append("X");
    } else if(message.m_is_correction.value_or(false) ||
        message.m_original_trade_id) {
      condition.m_type = TimeAndSale::Condition::Type::CORRECTION;
      append("U");
    } else if(message.m_opening_auction == std::string_view("O") ||
        message.m_market_state == std::string_view("Opening Trade")) {
      condition.m_type = TimeAndSale::Condition::Type::OPEN;
      append("O");
    } else if(message.m_opening_auction == std::string_view("R")) {
      condition.m_type = TimeAndSale::Condition::Type::REOPEN;
      append("R");
    } else if(message.m_is_market_on_close.value_or(false)) {
      condition.m_type = TimeAndSale::Condition::Type::CLOSE;
      append("C");
    } else if(message.m_action == "AuctionTradeIndividual") {
      condition.m_type = TimeAndSale::Condition::Type::AUCTION;
      append("A");
    }
    if(message.m_cross_type && *message.m_cross_type != "Regular") {
      static constexpr auto CODES =
        std::to_array<std::pair<std::string_view, std::string_view>>({
          {"Basis", "BA"}, {"Contgt", "CG"}, {"Intrnl", "I"}, {"NAV", "NV"},
          {"STS", "ST"}, {"VWAP", "V"}, {"NC", "NC"}, {"Intentional", "IC"},
          {"Derivative", "DR"}, {"CCP-Closing Price", "CP"}, {"CPP", "PP"}});
      append_mapped(*message.m_cross_type, CODES, "CX");
    }
    if(message.m_settlement_terms) {
      static constexpr auto CODES =
        std::to_array<std::pair<std::string_view, std::string_view>>({
          {"Cash", "CA"}, {"CT", "CT"}, {"MS", "MS"}, {"NN", "NN"},
          {"Future", "F"}, {"ND", "ND"}});
      auto is_date = message.m_settlement_terms->size() == 8 &&
        std::ranges::all_of(*message.m_settlement_terms,
          [] (auto c) { return c >= '0' && c <= '9'; });
      if(is_date) {
        append("DD");
      } else {
        append_mapped(*message.m_settlement_terms, CODES, "S");
      }
    }
    if(message.m_is_extended_hours.value_or(false)) {
      append("E");
    }
    if(message.m_is_bypass.value_or(false)) {
      append("B");
    }
    if(message.m_is_nonresident.value_or(false)) {
      append("N");
    }
    if(message.m_is_dark.value_or(false)) {
      append("D");
    }
    if(message.m_is_mid_only.value_or(false)) {
      append("M");
    }
    if(message.m_is_conditional.value_or(false)) {
      append("CO");
    }
    if(message.m_melo) {
      append("L");
    }
    if(message.m_purestream) {
      append("P");
    }
    if(condition.m_code.empty()) {
      condition.m_type = TimeAndSale::Condition::Type::REGULAR;
      condition.m_code = "@";
    }
    return condition;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  std::string TmxIpMarketDataFeedClient<C, D, T, M>::get_order_key(
      const Book& book, Side side, boost::optional<std::uint64_t> broker,
      std::string_view id) {
    auto key = std::string();
    if(book.m_venue == Venues::NEON) {
      if(side == Side::BID) {
        key = "B:";
      } else {
        key = "S:";
      }
    }
    if(book.m_venue == Venues::TSX || book.m_venue == Venues::TSXV ||
        book.m_venue == Venues::XATS || book.m_venue == Venues::ALX) {
      if(!broker) {
        return {};
      }
      key += std::to_string(*broker) + ':';
    }
    key += id;
    return key;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  Quantity TmxIpMarketDataFeedClient<C, D, T, M>::get_quantity(
      const Book& book, const OrderEntry& order) {
    if(book.m_board_lot <= 0 ||
        order.m_price.m_type != TmxIpPrice::Type::LIMIT ||
        order.m_price.m_value <= Money::ZERO) {
      return 0;
    }
    return floor_to(order.m_quantity, book.m_board_lot);
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  Venue TmxIpMarketDataFeedClient<C, D, T, M>::get_venue(
      const Ticker& ticker, const TmxIpMessageHeader& header) const {
    auto venue = m_config.m_venue;
    if(venue == Venues::CSE && ticker.get_venue() != Venues::CSE) {
      venue = Venues::PURE;
    }
    if(header.m_book_type &&
        *header.m_book_type != VENUES.from(venue).m_market_center) {
      return {};
    }
    return venue;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  typename TmxIpMarketDataFeedClient<C, D, T, M>::Book*
      TmxIpMarketDataFeedClient<C, D, T, M>::find_book(
        std::string_view symbol, const TmxIpMessageHeader& header) {
    if(!m_config.m_venue) {
      return nullptr;
    }
    auto info = find_ticker(symbol);
    if(!info || !m_open_state.is_open()) {
      return nullptr;
    }
    auto venue = get_venue(info->m_ticker, header);
    if(!venue) {
      return nullptr;
    }
    auto [i, is_inserted] = m_books.try_emplace(info->m_ticker);
    if(is_inserted) {
      i->second.m_ticker = info->m_ticker;
      i->second.m_venue = venue;
      i->second.m_mpid = VENUES.from(venue).m_display_name;
      i->second.m_prefix = std::string(symbol) + ':';
      i->second.m_board_lot = info->m_board_lot;
    }
    return &i->second;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  std::string TmxIpMarketDataFeedClient<C, D, T, M>::get_mpid(
      const Book& book, boost::optional<std::uint64_t> broker) const {
    if(book.m_venue != book.m_ticker.get_venue() || !broker) {
      return book.m_mpid;
    }
    return get_broker_name(broker);
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  std::string TmxIpMarketDataFeedClient<C, D, T, M>::get_broker_name(
      boost::optional<std::uint64_t> broker) const {
    if(!broker) {
      return {};
    }
    auto i = m_config.m_mpid_mappings.find(*broker);
    if(i != m_config.m_mpid_mappings.end()) {
      return i->second;
    }
    auto code = std::to_string(*broker);
    return std::string(3 - code.size(), '0') + code;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::update(Book& book,
      const std::string& key, OrderEntry order,
      boost::posix_time::ptime timestamp) {
    if(key.empty()) {
      return;
    }
    auto quantity = get_quantity(book, order);
    auto i = book.m_orders.find(key);
    auto previous_quantity = Quantity(0);
    auto previous_price = Money();
    auto is_broker_changed = false;
    if(i != book.m_orders.end()) {
      if(!order.m_broker) {
        order.m_broker = i->second.m_broker;
      }
      is_broker_changed = book.m_venue == book.m_ticker.get_venue() &&
        order.m_broker != i->second.m_broker;
      previous_quantity = get_quantity(book, i->second);
      previous_price = i->second.m_price.m_value;
    }
    if(order.m_quantity == 0) {
      remove(book, key, timestamp);
      return;
    }
    if(i != book.m_orders.end()) {
      i->second = order;
    } else {
      book.m_orders.emplace(key, order);
    }
    if(!is_broker_changed && quantity == previous_quantity &&
        (quantity == 0 || order.m_price.m_value == previous_price)) {
      return;
    }
    auto id = book.m_prefix + key;
    if(quantity == 0) {
      m_feed_client->remove_order(id, timestamp);
    } else {
      m_feed_client->add_order(book.m_ticker, book.m_venue,
        get_mpid(book, order.m_broker), false, id, order.m_side,
        order.m_price.m_value, quantity, timestamp);
    }
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::remove(Book& book,
      const std::string& key, boost::posix_time::ptime timestamp) {
    auto i = book.m_orders.find(key);
    if(i == book.m_orders.end()) {
      return;
    }
    auto quantity = get_quantity(book, i->second);
    book.m_orders.erase(i);
    if(quantity != 0) {
      m_feed_client->remove_order(book.m_prefix + key, timestamp);
    }
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::update(
      const TickerInfo& info, boost::posix_time::ptime timestamp) {
    auto& previous = m_tickers[info.m_ticker.get_symbol()];
    if(!previous || (previous->m_ticker == info.m_ticker &&
        previous->m_board_lot == info.m_board_lot)) {
      previous = info;
      return;
    }
    if(previous->m_ticker != info.m_ticker) {
      m_opening_quotes.erase(previous->m_ticker);
    }
    auto node = m_books.extract(previous->m_ticker);
    previous = info;
    if(node.empty()) {
      return;
    }
    auto& book = node.mapped();
    timestamp = venue_to_utc(book.m_venue, timestamp);
    for(auto& [key, order] : book.m_orders) {
      if(get_quantity(book, order) != 0) {
        m_feed_client->remove_order(book.m_prefix + key, timestamp);
      }
    }
    book.m_ticker = info.m_ticker;
    book.m_board_lot = info.m_board_lot;
    auto venue = get_venue(info.m_ticker, {});
    if(venue != book.m_venue) {
      book.m_venue = venue;
      book.m_mpid = VENUES.from(venue).m_display_name;
    }
    for(auto& [key, order] : book.m_orders) {
      auto quantity = get_quantity(book, order);
      if(quantity != 0) {
        m_feed_client->add_order(book.m_ticker, book.m_venue,
          get_mpid(book, order.m_broker), false, book.m_prefix + key,
          order.m_side, order.m_price.m_value, quantity, timestamp);
      }
    }
    node.key() = info.m_ticker;
    m_books.insert(std::move(node));
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish(
      const TmxIpSymbolStatus& message) {
    auto listing = Venue();
    if(message.m_listing_market) {
      auto code = *message.m_listing_market;
      if(code == "T") {
        listing = Venues::TSX;
      } else if(code == "V") {
        listing = Venues::TSXV;
      } else if(code == "N") {
        listing = Venues::CSE;
      } else if(code == "E") {
        listing = Venues::NEOE;
      } else if(code == "O") {
        listing = Venues::OMGA;
      } else {
        return;
      }
    }
    auto symbol = std::string(message.m_symbol);
    auto info = TickerInfo();
    auto i = m_tickers.find(symbol);
    if(i != m_tickers.end() && i->second) {
      info = *i->second;
    } else if(!listing || !message.m_name || !message.m_board_lot) {
      if(auto previous = find_ticker(message.m_symbol)) {
        info = *previous;
      }
    }
    if(listing) {
      info.m_ticker = Ticker(symbol, listing);
    } else if(!info.m_ticker.get_venue()) {
      return;
    }
    if(message.m_name) {
      info.m_name = *message.m_name;
    }
    if(message.m_board_lot) {
      info.m_board_lot = Quantity(*message.m_board_lot);
    }
    m_feed_client->add(info);
    update(info, message.m_header.m_trading_timestamp.value_or(
      message.m_header.m_timestamp));
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish(
      const TmxIpOrderBook& message) {
    for(auto& record : message.m_orders) {
      if(record.m_settlement_terms || record.m_is_nonresident.value_or(false)) {
        continue;
      }
      auto book = find_book(record.m_symbol, message.m_header);
      if(!book) {
        continue;
      }
      auto price = record.m_public_price.value_or(record.m_price.value_or(
        TmxIpPrice(TmxIpPrice::Type::MARKET, Money::ZERO)));
      auto key = get_order_key(*book, record.m_side, record.m_broker,
        record.m_order_id);
      auto timestamp = venue_to_utc(book->m_venue,
        message.m_header.m_trading_timestamp.value_or(
          message.m_header.m_timestamp));
      update(*book, key, OrderEntry(record.m_side, price, record.m_quantity,
        price.m_type != TmxIpPrice::Type::LIMIT ||
          price.m_value == Money::ZERO, record.m_broker), timestamp);
    }
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish(
      const TmxIpOrderCancelReport& message) {
    if(message.m_settlement_terms || message.m_is_nonresident.value_or(false)) {
      return;
    }
    auto book = find_book(message.m_symbol, message.m_header);
    if(!book) {
      return;
    }
    auto side = Side::BID;
    if(message.m_action == "Sell") {
      side = Side::ASK;
    }
    auto id = std::string();
    if(book->m_venue == Venues::NEON) {
      id = boost::lexical_cast<std::string>(message.m_public_price.m_value);
    } else if(message.m_order_id) {
      id = *message.m_order_id;
    } else {
      return;
    }
    auto key = get_order_key(*book, side, message.m_broker, id);
    auto timestamp = venue_to_utc(book->m_venue,
      message.m_header.m_trading_timestamp.value_or(
        message.m_header.m_timestamp));
    if(message.m_confirmation == "Booked") {
      if(message.m_previous_order_id &&
          message.m_previous_order_id != message.m_order_id) {
        remove(*book, get_order_key(*book, side, message.m_broker,
          *message.m_previous_order_id), timestamp);
      }
      if(book->m_venue == Venues::NEON && message.m_previous_price &&
          message.m_previous_price->m_value != message.m_public_price.m_value) {
        remove(*book, get_order_key(*book, side, message.m_broker,
          boost::lexical_cast<std::string>(
            message.m_previous_price->m_value)), timestamp);
      }
      update(*book, key, OrderEntry(side, message.m_public_price,
        message.m_quantity,
        message.m_public_price.m_type != TmxIpPrice::Type::LIMIT ||
          message.m_public_price.m_value == Money::ZERO, message.m_broker),
        timestamp);
    } else if(message.m_confirmation == "Cancelled" &&
        (book->m_venue == Venues::CHIC || book->m_venue == Venues::XCX2 ||
          book->m_venue == Venues::OMGA || book->m_venue == Venues::LYNX)) {
      auto i = book->m_orders.find(key);
      if(i != book->m_orders.end()) {
        auto order = i->second;
        if(book->m_venue == Venues::OMGA || book->m_venue == Venues::LYNX) {
          if(message.m_quantity == 0) {
            order.m_quantity = 0;
          } else {
            order.m_quantity = std::max(
              Quantity(0), order.m_quantity - Quantity(message.m_quantity));
          }
        } else {
          order.m_quantity = message.m_quantity;
        }
        update(*book, key, order, timestamp);
      }
    } else if(message.m_confirmation == "Cancelled" ||
        message.m_confirmation == "Killed") {
      remove(*book, key, timestamp);
    } else if(message.m_confirmation == "PriceAssigned") {
      auto i = book->m_orders.find(key);
      if(i != book->m_orders.end()) {
        auto order = i->second;
        order.m_price = message.m_public_price;
        order.m_is_market = order.m_price.m_type != TmxIpPrice::Type::LIMIT ||
          order.m_price.m_value == Money::ZERO;
        update(*book, key, order, timestamp);
      }
    }
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish(
      const TmxIpClearOrderBook& message) {
    auto book = find_book(message.m_symbol, message.m_header);
    if(!book) {
      return;
    }
    auto timestamp = venue_to_utc(book->m_venue,
      message.m_header.m_trading_timestamp.value_or(
        message.m_header.m_timestamp));
    while(!book->m_orders.empty()) {
      auto key = book->m_orders.begin()->first;
      remove(*book, key, timestamp);
    }
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish(
      const TmxIpTradeReport& message) {
    if(!m_config.m_venue) {
      publish_trade(message);
      return;
    }
    if((message.m_action != "Trade" &&
        message.m_action != "AuctionTradeIndividual") ||
        message.m_is_correction.value_or(false) ||
        message.m_original_trade_id ||
        message.m_settlement_terms ||
        message.m_is_nonresident.value_or(false)) {
      return;
    }
    auto book = find_book(message.m_symbol, message.m_header);
    if(!book) {
      return;
    }
    auto timestamp = venue_to_utc(book->m_venue,
      message.m_header.m_trading_timestamp.value_or(
        message.m_header.m_timestamp));
    for(auto i = std::size_t(0); i != message.m_sides.size(); ++i) {
      auto& record = message.m_sides[i];
      auto side = Side::BID;
      if(i == 1) {
        side = Side::ASK;
      }
      auto id = std::string();
      if(book->m_venue == Venues::NEON) {
        id = boost::lexical_cast<std::string>(message.m_price.m_value);
      } else if(record.m_order_id) {
        id = *record.m_order_id;
      } else {
        continue;
      }
      auto key = get_order_key(*book, side, record.m_broker, id);
      auto j = book->m_orders.find(key);
      if(j == book->m_orders.end()) {
        continue;
      }
      auto order = j->second;
      if(record.m_display_quantity) {
        order.m_quantity = *record.m_display_quantity;
      } else {
        order.m_quantity = std::max(Quantity(0),
          order.m_quantity - Quantity(message.m_quantity));
      }
      update(*book, key, order, timestamp);
    }
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish_trade(
      const TmxIpTradeReport& message) {
    if((message.m_action != "Trade" &&
        message.m_action != "Cancelled" &&
        message.m_action != "AuctionTradeIndividual") ||
        message.m_quantity == 0 ||
        message.m_price.m_type != TmxIpPrice::Type::LIMIT ||
        !message.m_header.m_exchange) {
      return;
    }
    auto market_center = std::string(*message.m_header.m_exchange);
    auto venue = from_market_center(market_center).m_venue;
    if(!venue) {
      return;
    }
    if((venue == Venues::NEOE || venue == Venues::NEON) &&
        message.m_header.m_book_type) {
      auto book = *message.m_header.m_book_type;
      if(book == "AQN") {
        venue = Venues::NEON;
      } else if(book == "AQL" || book == "AQD" || book == "AQS" ||
          book == "AQC") {
        venue = Venues::NEOE;
      } else {
        return;
      }
      market_center = book;
    }
    auto info = find_ticker(message.m_symbol);
    if(!info || !m_open_state.is_open()) {
      return;
    }
    if(message.m_action == "AuctionTradeIndividual" &&
        !message.m_is_correction.value_or(false) &&
        !message.m_original_trade_id) {
      auto is_lit = venue == Venues::NEOE &&
        (!message.m_header.m_book_type ||
          *message.m_header.m_book_type == "AQL");
      if(!is_lit || !message.m_trade_id) {
        return;
      }
      auto date = message.m_header.m_trading_timestamp->date();
      if(date != m_auction_date) {
        m_auction_date = date;
        m_auction_trades.clear();
      }
      auto key = std::string(message.m_symbol) + ':' +
        std::string(*message.m_trade_id);
      if(!m_auction_trades.insert(std::move(key)).second) {
        return;
      }
    }
    auto timestamp = *message.m_header.m_trading_timestamp;
    auto trade_timestamp = boost::optional<boost::posix_time::ptime>();
    for(auto& side : message.m_sides) {
      if(side.m_trade_timestamp &&
          (!trade_timestamp || *side.m_trade_timestamp < *trade_timestamp)) {
        trade_timestamp = side.m_trade_timestamp;
      }
    }
    if(trade_timestamp) {
      timestamp = *trade_timestamp;
    }
    timestamp = venue_to_utc(venue, timestamp);
    m_feed_client->publish(TickerTimeAndSale(TimeAndSale(timestamp,
      message.m_price.m_value, message.m_quantity, get_condition(message),
      market_center, get_broker_name(message.m_sides[0].m_broker),
      get_broker_name(message.m_sides[1].m_broker)), info->m_ticker));
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::update_orders(
      const TmxIpMbxMessage& message) {
    auto book = find_book(message.m_symbol, message.m_header);
    if(!book) {
      return;
    }
    auto timestamp = venue_to_utc(book->m_venue,
      message.m_header.m_trading_timestamp.value_or(
        message.m_header.m_timestamp));
    if(book->m_venue == Venues::NEOE && message.m_action == "AssignCOP") {
      for(auto& [key, entry] : book->m_orders) {
        if(entry.m_is_market) {
          auto order = entry;
          order.m_price = message.m_calculated_opening_price;
          update(*book, key, order, timestamp);
        }
      }
    } else if(book->m_venue == Venues::TSX || book->m_venue == Venues::TSXV) {
      for(auto& record : message.m_orders) {
        if(!record.m_key) {
          continue;
        }
        auto separator = record.m_key->find('|');
        if(separator == std::string_view::npos) {
          continue;
        }
        auto broker = std::uint64_t();
        auto first = record.m_key->data();
        auto last = first + separator;
        auto result = std::from_chars(first, last, broker);
        if(result.ec != std::errc() || result.ptr != last) {
          continue;
        }
        auto price = message.m_calculated_opening_price;
        if(message.m_action == "AssignLimit") {
          if(!record.m_price) {
            continue;
          }
          price = *record.m_price;
        }
        auto key = get_order_key(
          *book, Side::NONE, broker, record.m_key->substr(separator + 1));
        auto i = book->m_orders.find(key);
        if(i != book->m_orders.end()) {
          auto order = i->second;
          order.m_price = price;
          update(*book, key, order, timestamp);
        }
      }
    }
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::load_tickers() {
    auto query = TickerInfoQuery();
    query.set_index(Countries::CA);
    query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    for(auto& info : m_market_data_client->query(query)) {
      auto [i, is_inserted] =
        m_tickers.try_emplace(info.m_ticker.get_symbol(), info);
      if(!is_inserted && i->second && i->second->m_ticker != info.m_ticker) {
        i->second = boost::none;
      }
    }
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  const TickerInfo* TmxIpMarketDataFeedClient<C, D, T, M>::find_ticker(
      std::string_view symbol) {
    auto [i, is_inserted] = m_tickers.try_emplace(std::string(symbol));
    if(is_inserted) {
      try {
        auto query = TickerInfoQuery();
        query.set_index(Countries::CA);
        query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
        auto ticker = TickerAccessor(Beam::MemberAccessExpression("ticker",
          typeid(Ticker), Beam::ParameterExpression(0, typeid(TickerInfo))));
        query.set_filter(ticker.get_symbol() == std::string(symbol));
        for(auto& info : m_market_data_client->query(query)) {
          if(info.m_ticker.get_symbol() != symbol) {
            continue;
          }
          if(i->second && i->second->m_ticker != info.m_ticker) {
            i->second = boost::none;
            break;
          }
          i->second = info;
        }
      } catch(const std::exception&) {
        i->second = boost::none;
      }
    }
    if(!i->second) {
      return nullptr;
    }
    return &*i->second;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  bool TmxIpMarketDataFeedClient<C, D, T, M>::is_eligible(
      Venue venue, bool is_opening, boost::posix_time::ptime timestamp) {
    auto date = utc_to_venue(venue, timestamp).date();
    auto& session = m_sessions[venue];
    if(session.m_date != date) {
      session = Session();
      session.m_date = date;
      for(auto& event : m_schedule.find(timestamp, venue)) {
        if(event.m_code == "OPEN") {
          session.m_opening = event.m_timestamp;
        } else if(event.m_code == "CLOSE") {
          session.m_closing = event.m_timestamp;
        }
      }
    }
    if(!session.m_opening || !session.m_closing) {
      return false;
    }
    if(is_opening) {
      return timestamp < *session.m_opening;
    }
    return timestamp >= *session.m_opening && timestamp < *session.m_closing;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  typename TmxIpMarketDataFeedClient<C, D, T, M>::OpeningQuote*
      TmxIpMarketDataFeedClient<C, D, T, M>::find_opening_quote(
        std::string_view symbol, const TmxIpMessageHeader& header) {
    auto info = find_ticker(symbol);
    if(!info || get_venue(info->m_ticker, header) !=
        info->m_ticker.get_venue()) {
      return nullptr;
    }
    auto now = m_time_client->get_time();
    auto venue = info->m_ticker.get_venue();
    auto date = utc_to_venue(venue, now).date();
    auto timestamp = header.m_trading_timestamp.value_or(header.m_timestamp);
    if(timestamp.date() != date || !is_eligible(venue, true, now)) {
      return nullptr;
    }
    auto& quote = m_opening_quotes[info->m_ticker];
    if(quote.m_date != date) {
      quote = OpeningQuote();
      quote.m_date = date;
    }
    return &quote;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish(
      const TmxIpMbxMessage& message) {
    update_orders(message);
    if(message.m_action != "AssignCOP" ||
        message.m_calculated_opening_price.m_type != TmxIpPrice::Type::LIMIT) {
      return;
    }
    auto quote = find_opening_quote(message.m_symbol, message.m_header);
    if(!quote) {
      return;
    }
    auto price = message.m_calculated_opening_price.m_value;
    quote->m_price = price;
    if(message.m_paired_quantity) {
      quote->m_paired_quantity = *message.m_paired_quantity;
    } else if(message.m_theoretical_opening_quantity) {
      quote->m_paired_quantity = *message.m_theoretical_opening_quantity;
    }
    if(message.m_imbalance_side) {
      if(*message.m_imbalance_side == "BuySide") {
        quote->m_imbalance_side = Side::BID;
      } else if(*message.m_imbalance_side == "SellSide") {
        quote->m_imbalance_side = Side::ASK;
      } else {
        quote->m_imbalance_side = Side::NONE;
      }
    }
    if(message.m_imbalance_quantity) {
      quote->m_imbalance_quantity = *message.m_imbalance_quantity;
    }
    publish_opening_quote(message.m_symbol, message.m_header);
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish(
      const TmxIpOpeningAuction& message) {
    auto is_numeric = !message.m_calculated_opening_price ||
      message.m_calculated_opening_price->m_type == TmxIpPrice::Type::LIMIT;
    if(!message.m_symbol || !is_numeric) {
      return;
    }
    auto quote = find_opening_quote(*message.m_symbol, message.m_header);
    if(!quote) {
      return;
    }
    if(message.m_calculated_opening_price) {
      auto price = message.m_calculated_opening_price->m_value;
      quote->m_price = price;
    }
    if(message.m_action == "PairedVolume" && message.m_paired_quantity) {
      quote->m_paired_quantity = *message.m_paired_quantity;
    } else if(message.m_action == "OddlotImbalance") {
      if(message.m_imbalance_side) {
        quote->m_imbalance_side = *message.m_imbalance_side;
      }
      if(message.m_imbalance_quantity) {
        quote->m_imbalance_quantity = *message.m_imbalance_quantity;
      }
    }
    publish_opening_quote(*message.m_symbol, message.m_header);
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish(
      const TmxIpCbboQuote& message) {
    auto info = find_ticker(message.m_symbol);
    if(!info || !m_open_state.is_open()) {
      return;
    }
    auto now = m_time_client->get_time();
    auto venue = info->m_ticker.get_venue();
    if(!is_eligible(venue, false, now)) {
      return;
    }
    auto timestamp = now;
    auto source_timestamp = message.m_outbound_timestamp;
    if(!source_timestamp) {
      source_timestamp = message.m_publication_timestamp;
    }
    if(source_timestamp) {
      if(source_timestamp->date() != utc_to_venue(venue, now).date()) {
        return;
      }
      timestamp = venue_to_utc(venue, *source_timestamp);
    }
    auto& bid = message.m_sides[0];
    auto& ask = message.m_sides[1];
    m_feed_client->publish(TickerBboQuote(
      BboQuote(Quote(bid.m_price, bid.m_quantity, Side::BID),
        Quote(ask.m_price, ask.m_quantity, Side::ASK), timestamp),
      info->m_ticker));
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::publish_opening_quote(
      std::string_view symbol, const TmxIpMessageHeader& header) {
    auto& info = *m_tickers.at(std::string(symbol));
    auto& opening = m_opening_quotes.at(info.m_ticker);
    if(!opening.m_price || !m_open_state.is_open()) {
      return;
    }
    if(!is_eligible(
        info.m_ticker.get_venue(), true, m_time_client->get_time())) {
      return;
    }
    auto bid = opening.m_paired_quantity;
    auto ask = opening.m_paired_quantity;
    if(opening.m_imbalance_side == Side::BID) {
      bid += opening.m_imbalance_quantity;
    } else if(opening.m_imbalance_side == Side::ASK) {
      ask += opening.m_imbalance_quantity;
    }
    auto timestamp = venue_to_utc(info.m_ticker.get_venue(),
      header.m_trading_timestamp.value_or(header.m_timestamp));
    m_feed_client->publish(
      TickerBboQuote(BboQuote(Quote(*opening.m_price, bid, Side::BID),
        Quote(*opening.m_price, ask, Side::ASK), timestamp), info.m_ticker));
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::log_error(
      const StampMessage& message, const std::exception& error) {
    if(!m_config.m_is_logging_messages) {
      log(message);
    }
    auto out = std::stringstream();
    out << "(bad_message " << m_time_client->get_time() << ' ' <<
      error.what() << ")\n";
    std::cout << out.str() << std::flush;
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::read_loop() {
    try {
      load_tickers();
      auto session = std::uint64_t();
      while(m_open_state.is_open()) {
        auto next_session = std::uint64_t();
        auto message = m_tmx_ip_client->read(Beam::out(next_session));
        if(!m_open_state.is_open()) {
          break;
        }
        if(next_session != session) {
          auto timestamp = m_time_client->get_time();
          for(auto& [key, book] : m_books) {
            while(!book.m_orders.empty()) {
              auto id = book.m_orders.begin()->first;
              remove(book, id, timestamp);
            }
          }
          m_books.clear();
          m_opening_quotes.clear();
          m_auction_date = boost::gregorian::date();
          m_auction_trades.clear();
          session = next_session;
        }
        if(m_config.m_is_logging_messages) {
          log(message);
        }
        try {
          visit(message,
            [&] (const TmxIpSymbolStatus& message) { publish(message); },
            [&] (const TmxIpOrderBook& message) { publish(message); },
            [&] (const TmxIpOrderCancelReport& message) { publish(message); },
            [&] (const TmxIpClearOrderBook& message) { publish(message); },
            [&] (const TmxIpTradeReport& message) { publish(message); },
            [&] (const TmxIpMbxMessage& message) { publish(message); },
            [&] (const TmxIpOpeningAuction& message) { publish(message); },
            [&] (const TmxIpCbboQuote& message) { publish(message); },
            [] (const auto&) {});
        } catch(const TmxIpParserException& e) {
          log_error(message, e);
        } catch(const StampParserException& e) {
          log_error(message, e);
        } catch(const std::exception&) {
          if(!m_config.m_is_logging_messages) {
            log(message);
          }
          throw;
        }
      }
    } catch(const std::exception&) {
      if(m_open_state.is_open()) {
        m_exception = std::current_exception();
      }
    }
    m_is_finished = true;
  }
}

#endif
