#ifndef TMX_IP_MARKET_DATA_FEED_CLIENT_HPP
#define TMX_IP_MARKET_DATA_FEED_CLIENT_HPP
#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/IO/OpenState.hpp>
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
   * Publishes listing-venue opening prices and consolidated best quotes.
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
      struct OpeningQuote {
        boost::gregorian::date m_date;
        boost::optional<Money> m_price;
        Quantity m_paired_quantity;
        Side m_imbalance_side = Side::NONE;
        Quantity m_imbalance_quantity;
      };
      TmxIpConfiguration m_config;
      TradingSchedule m_schedule;
      Beam::local_ptr_t<C> m_tmx_ip_client;
      Beam::local_ptr_t<D> m_market_data_client;
      Beam::local_ptr_t<T> m_time_client;
      Beam::local_ptr_t<M> m_feed_client;
      std::unordered_map<std::string, boost::optional<TickerInfo>> m_tickers;
      std::unordered_map<Ticker, OpeningQuote> m_opening_quotes;
      Beam::Sync<std::exception_ptr> m_exception;
      std::atomic_bool m_is_finished;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      static void log(const StampMessage& message);
      static Venue get_venue(std::string_view exchange);
      TmxIpMarketDataFeedClient(const TmxIpMarketDataFeedClient&) = delete;
      TmxIpMarketDataFeedClient& operator =(
        const TmxIpMarketDataFeedClient&) = delete;
      void load_tickers();
      const TickerInfo* find_ticker(std::string_view symbol);
      bool is_eligible(Venue venue, bool is_opening,
        boost::posix_time::ptime timestamp) const;
      OpeningQuote* find_opening_quote(
        std::string_view symbol, const TmxIpMessageHeader& header);
      void publish(const TmxIpMbxMessage& message);
      void publish(const TmxIpOpeningAuction& message);
      void publish(const TmxIpCbboQuote& message);
      void publish_opening_quote(
        std::string_view symbol, const TmxIpMessageHeader& header);
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
  Venue TmxIpMarketDataFeedClient<C, D, T, M>::get_venue(
      std::string_view exchange) {
    if(exchange == "TSE") {
      return Venues::TSX;
    } else if(exchange == "CDX") {
      return Venues::TSXV;
    } else if(exchange == "CNQ") {
      return Venues::CSE;
    } else if(exchange == "AQL") {
      return Venues::NEOE;
    }
    return {};
  }

  template<typename C, typename D, typename T, typename M> requires
    IsTmxIpClient<Beam::dereference_t<C>> &&
      IsMarketDataClient<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
      IsMarketDataFeedClient<Beam::dereference_t<M>>
  void TmxIpMarketDataFeedClient<C, D, T, M>::load_tickers() {
    auto query = TickerInfoQuery();
    query.set_index(m_config.m_country);
    query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    for(auto& info : m_market_data_client->query(query)) {
      if(VENUES.from(info.m_ticker.get_venue()).m_country_code !=
          m_config.m_country) {
        continue;
      }
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
        query.set_index(m_config.m_country);
        query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
        auto ticker = TickerAccessor(Beam::MemberAccessExpression("ticker",
          typeid(Ticker), Beam::ParameterExpression(0, typeid(TickerInfo))));
        query.set_filter(ticker.get_symbol() == std::string(symbol));
        for(auto& info : m_market_data_client->query(query)) {
          if(info.m_ticker.get_symbol() != symbol ||
              VENUES.from(info.m_ticker.get_venue()).m_country_code !=
                m_config.m_country) {
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
      Venue venue, bool is_opening, boost::posix_time::ptime timestamp) const {
    auto opening = boost::optional<boost::posix_time::ptime>();
    auto closing = boost::optional<boost::posix_time::ptime>();
    for(auto& event : m_schedule.find(timestamp, venue)) {
      if(event.m_code == "OPEN") {
        opening = event.m_timestamp;
      } else if(event.m_code == "CLOSE") {
        closing = event.m_timestamp;
      }
    }
    if(!opening || !closing) {
      return false;
    }
    if(is_opening) {
      return timestamp < *opening;
    }
    return timestamp >= *opening && timestamp < *closing;
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
    if(!info || !header.m_exchange ||
        get_venue(*header.m_exchange) != info->m_ticker.get_venue()) {
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
    if(message.m_action != "AssignCOP" ||
        message.m_calculated_opening_price.m_type != TmxIpPrice::Type::LIMIT) {
      return;
    }
    auto quote = find_opening_quote(message.m_symbol, message.m_header);
    if(!quote) {
      return;
    }
    auto price = message.m_calculated_opening_price.m_value;
    if(quote->m_price && quote->m_price != price) {
      quote->m_paired_quantity = 0;
      quote->m_imbalance_side = Side::NONE;
      quote->m_imbalance_quantity = 0;
    }
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
      if(quote->m_price && quote->m_price != price) {
        quote->m_paired_quantity = 0;
        quote->m_imbalance_side = Side::NONE;
        quote->m_imbalance_quantity = 0;
      }
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
  void TmxIpMarketDataFeedClient<C, D, T, M>::read_loop() {
    try {
      load_tickers();
      while(m_open_state.is_open()) {
        auto message = m_tmx_ip_client->read();
        if(!m_open_state.is_open()) {
          break;
        }
        if(m_config.m_is_logging_messages) {
          log(message);
        }
        visit(message,
          [&] (const TmxIpMbxMessage& message) { publish(message); },
          [&] (const TmxIpOpeningAuction& message) { publish(message); },
          [&] (const TmxIpCbboQuote& message) { publish(message); },
          [] (const auto&) {});
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
