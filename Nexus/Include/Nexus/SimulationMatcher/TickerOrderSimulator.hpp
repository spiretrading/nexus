#ifndef NEXUS_TICKER_ORDER_SIMULATOR_HPP
#define NEXUS_TICKER_ORDER_SIMULATOR_HPP
#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/FixTags.hpp"
#include "Nexus/Definitions/StandardDestinations.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"
#include "Nexus/MarketDataService/TickerSnapshot.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/SimulationMatcher/SimulationExecutionReportQueue.hpp"

namespace Nexus {
namespace Details {
  struct PostingVenue {
    Destination m_destination;
    Venue m_listing_venue;
    Venue m_venue;
  };

  inline const std::vector<PostingVenue>& get_posting_venues() {
    static const auto venues = std::vector<PostingVenue>({
      {Destinations::ALPHA, Venues::TSX, Venues::XATS},
      {Destinations::ALPHA, Venues::TSXV, Venues::XATS},
      {Destinations::ASXT, Venues::ASX, Venues::ASX},
      {Destinations::CHIX, Venues::NEOE, Venues::CHIC},
      {Destinations::CHIX, Venues::CSE, Venues::CHIC},
      {Destinations::CHIX, Venues::TSX, Venues::CHIC},
      {Destinations::CHIX, Venues::TSXV, Venues::CHIC},
      {Destinations::CSE, Venues::CSE, Venues::CSE},
      {Destinations::CSE2, Venues::CSE, Venues::CSE2},
      {Destinations::CSE2, Venues::TSX, Venues::CSE2},
      {Destinations::CSE2, Venues::TSXV, Venues::CSE2},
      {Destinations::CX2, Venues::NEOE, Venues::XCX2},
      {Destinations::CX2, Venues::CSE, Venues::XCX2},
      {Destinations::CX2, Venues::TSX, Venues::XCX2},
      {Destinations::CX2, Venues::TSXV, Venues::XCX2},
      {Destinations::CXA, Venues::ASX, Venues::CXA},
      {Destinations::LYNX, Venues::NEOE, Venues::LYNX},
      {Destinations::LYNX, Venues::CSE, Venues::LYNX},
      {Destinations::LYNX, Venues::TSX, Venues::LYNX},
      {Destinations::LYNX, Venues::TSXV, Venues::LYNX},
      {Destinations::MATNLP, Venues::NEOE, Venues::MATN},
      {Destinations::MATNLP, Venues::CSE, Venues::MATN},
      {Destinations::MATNLP, Venues::TSX, Venues::MATN},
      {Destinations::MATNLP, Venues::TSXV, Venues::MATN},
      {Destinations::MATNMF, Venues::NEOE, Venues::MATN},
      {Destinations::MATNMF, Venues::CSE, Venues::MATN},
      {Destinations::MATNMF, Venues::TSX, Venues::MATN},
      {Destinations::MATNMF, Venues::TSXV, Venues::MATN},
      {Destinations::NEOE, Venues::NEOE, Venues::NEOE},
      {Destinations::NEOE, Venues::CSE, Venues::NEOE},
      {Destinations::NEOE, Venues::TSX, Venues::NEOE},
      {Destinations::NEOE, Venues::TSXV, Venues::NEOE},
      {Destinations::OMEGA, Venues::NEOE, Venues::OMGA},
      {Destinations::OMEGA, Venues::CSE, Venues::OMGA},
      {Destinations::OMEGA, Venues::TSX, Venues::OMGA},
      {Destinations::OMEGA, Venues::TSXV, Venues::OMGA},
      {Destinations::PURE, Venues::TSX, Venues::PURE},
      {Destinations::PURE, Venues::TSXV, Venues::PURE},
      {Destinations::TSX, Venues::TSX, Venues::TSX},
      {Destinations::TSX, Venues::TSXV, Venues::TSXV}});
    return venues;
  }

  inline Venue get_posting_venue(
      const Destination& destination, Venue listing_venue) {
    for(auto& entry : get_posting_venues()) {
      if(entry.m_destination == destination &&
          entry.m_listing_venue == listing_venue) {
        return entry.m_venue;
      }
    }
    return Venue();
  }
}

  /**
   * Handles simulating Orders submitted for a specific Ticker.
   * @param <T> The type of TimeClient used for Order timestamps.
   */
  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  class TickerOrderSimulator {
    public:

      /** The type of TimeClient used for Order timestamps. */
      using TimeClient = Beam::dereference_t<T>;

      /**
       * Constructs a TickerOrderSimulator.
       * @param ticker The Ticker to simulate Order executions for.
       * @param time_client The TimeClient used for Order timestamps.
       * @param reports The queue to publish ExecutionReports through.
       */
      template<Beam::Initializes<T> TF>
      TickerOrderSimulator(const Ticker& ticker, TF&& time_client,
        std::shared_ptr<SimulationExecutionReportQueue> reports);

      /**
       * Initializes the simulation with a snapshot of the Ticker's market data,
       * discarding the book and quote it held.
       * @param snapshot The snapshot to initialize the simulation with.
       */
      void initialize(const TickerSnapshot& snapshot);

      /**
       * Recovers a previously submitted Order.
       * @param order The Order to recover.
       */
      void recover(const std::shared_ptr<PrimitiveOrder>& order);

      /**
       * Submits an Order for simulated Order entry.
       * @param order The Order to submit.
       */
      void submit(const std::shared_ptr<PrimitiveOrder>& order);

      /**
       * Cancels a previously submitted order.
       * @param order The Order to cancel.
       */
      void cancel(const std::shared_ptr<PrimitiveOrder>& order);

      /**
       * Updates an Order.
       * @param order The Order to update.
       * @param report The ExecutionReport containing the update.
       */
      void update(const std::shared_ptr<PrimitiveOrder>& order,
        const ExecutionReport& report);

      /**
       * Updates the simulation with a BboQuote.
       * @param bbo The BboQuote to update the simulation with.
       */
      void update(const BboQuote& bbo);

      /**
       * Updates the simulation with a TimeAndSale.
       * @param time_and_sale The TimeAndSale to update the simulation with.
       */
      void update(const TimeAndSale& time_and_sale);

      /**
       * Updates the simulation with a BookQuote.
       * @param book_quote The BookQuote to update the simulation with.
       */
      void update(const BookQuote& book_quote);

    private:
      using Level = std::tuple<Venue, Side, Money>;
      using Listing = std::tuple<Venue, Side, Money, std::string>;
      struct OrderEntry {
        OrderStatus m_status;
        Quantity m_remaining_quantity;
        Venue m_venue;
        Quantity m_queue_quantity;
        Quantity m_trailing_quantity;
        Venue m_session_venue;
      };
      struct Session {
        boost::posix_time::ptime m_pre_open;
        boost::posix_time::ptime m_open;
        boost::posix_time::ptime m_close;
      };
      enum class Phase {
        CONTINUOUS,
        PRE_OPEN,
        OPENING
      };
      struct PeggedOrderEntry {
        std::string m_exec_inst;
        Money m_peg_difference;
        Money m_effective_price;
      };
      static constexpr auto REGULAR_SALE = std::string_view("@");
      static constexpr auto PRE_OPEN_EVENT = std::string_view("PRE_OPEN");
      static constexpr auto OPEN_EVENT = std::string_view("OPEN");
      static constexpr auto CLOSE_EVENT = std::string_view("CLOSE");
      static constexpr auto BOARD_LOT = 100;
      static inline const auto CLOSING_PERIOD = boost::posix_time::minutes(15);
      Beam::local_ptr_t<T> m_time_client;
      std::shared_ptr<SimulationExecutionReportQueue> m_reports;
      Ticker m_ticker;
      boost::gregorian::date m_date;
      boost::posix_time::ptime m_session_timestamp;
      Session m_session;
      std::unordered_map<Venue, Session> m_sessions;
      Phase m_phase;
      boost::posix_time::ptime m_cross_time;
      boost::posix_time::ptime m_uncross_time;
      boost::posix_time::ptime m_venue_close_time;
      boost::posix_time::ptime m_closing_cross_time;
      bool m_is_closed;
      std::vector<std::shared_ptr<PrimitiveOrder>> m_orders;
      std::unordered_map<OrderId, OrderEntry> m_entries;
      std::unordered_map<OrderId, PeggedOrderEntry> m_pegged_entries;
      std::map<Listing, Quantity> m_listings;
      std::map<Level, Quantity> m_levels;
      BboQuote m_bbo;
      std::mutex m_mutex;

      TickerOrderSimulator(const TickerOrderSimulator&) = delete;
      TickerOrderSimulator& operator =(const TickerOrderSimulator&) = delete;
      void set_session_timestamps(boost::posix_time::ptime timestamp);
      boost::gregorian::date get_session_date(
        boost::posix_time::ptime timestamp) const;
      const Session& load_session(Venue venue);
      Phase get_phase(boost::posix_time::ptime timestamp) const;
      bool is_crossed() const;
      bool is_held(const OrderEntry& entry);
      bool update_session(
        boost::posix_time::ptime timestamp, bool is_listing_print = false);
      bool is_opening_cross(const TimeAndSale& time_and_sale, bool is_listing);
      bool is_trade(const TimeAndSale::Condition& condition) const;
      bool expire_market_on_close_orders();
      bool expire_market_on_close_orders(boost::posix_time::ptime timestamp);
      void evaluate_orders();
      void evaluate_held_orders();
      void cross(const TimeAndSale& time_and_sale, bool is_closing);
      bool cross(const TimeAndSale& time_and_sale, Side side, Quantity size,
        bool is_closing);
      void submit_pegged(const PrimitiveOrder& order);
      void enqueue(const std::shared_ptr<PrimitiveOrder>& order,
        OrderStatus status, boost::posix_time::ptime timestamp,
        Quantity last_quantity, Money last_price, std::string text);
      void enqueue(const std::shared_ptr<PrimitiveOrder>& order,
        OrderStatus status, boost::posix_time::ptime timestamp,
        Quantity last_quantity, Money last_price);
      OrderStatus fill(const std::shared_ptr<PrimitiveOrder>& order,
        Money price, Quantity quantity);
      OrderStatus fill(
        const std::shared_ptr<PrimitiveOrder>& order, Money price);
      OrderStatus evaluate(
        const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status);
      OrderStatus update_pegged(
        const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status);
      OrderEntry make_entry(const PrimitiveOrder& order, OrderStatus status,
        Quantity remaining_quantity);
      Quantity apply(const BookQuote& book_quote);
      void advance(const Level& level, Quantity delta);
      Quantity allocate(
        const std::shared_ptr<PrimitiveOrder>& order, Quantity available);
      bool match(const TimeAndSale& time_and_sale, Side side, Quantity size);
      void match(const TimeAndSale& time_and_sale);
      void erase(const std::shared_ptr<PrimitiveOrder>& order);
  };

  template<typename T>
  TickerOrderSimulator(
    const Ticker&, T&&, std::shared_ptr<SimulationExecutionReportQueue>) ->
      TickerOrderSimulator<std::remove_cvref_t<T>>;

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  TickerOrderSimulator<T>::TickerOrderSimulator(const Ticker& ticker,
      TF&& time_client, std::shared_ptr<SimulationExecutionReportQueue> reports)
      : m_time_client(std::forward<TF>(time_client)),
        m_reports(std::move(reports)),
        m_ticker(ticker) {
    set_session_timestamps(m_time_client->get_time());
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::initialize(const TickerSnapshot& snapshot) {
    auto lock = std::lock_guard(m_mutex);
    m_listings.clear();
    m_levels.clear();
    for(auto& quote : snapshot.m_bids) {
      apply(*quote);
    }
    for(auto& quote : snapshot.m_asks) {
      apply(*quote);
    }
    m_bbo = *snapshot.m_bbo_quote;
    m_phase = get_phase(m_time_client->get_time());
    m_cross_time = boost::posix_time::not_a_date_time;
    m_uncross_time = boost::posix_time::not_a_date_time;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::recover(
      const std::shared_ptr<PrimitiveOrder>& order) {
    auto lock = std::lock_guard(m_mutex);
    auto [status, remaining] = order->with(
      [&] (auto status, const auto& reports) {
        auto remaining = order->get_info().m_fields.m_quantity;
        for(auto& report : reports) {
          remaining -= report.m_last_quantity;
        }
        return std::pair(status, remaining);
      });
    if(is_terminal(status)) {
      return;
    }
    if(update_session(m_time_client->get_time())) {
      evaluate_held_orders();
    }
    m_entries[order->get_info().m_id] = make_entry(*order, status, remaining);
    auto next_status = evaluate(order, status);
    if(is_terminal(next_status)) {
      m_pegged_entries.erase(order->get_info().m_id);
      m_entries.erase(order->get_info().m_id);
      return;
    }
    m_orders.push_back(order);
    m_entries[order->get_info().m_id].m_status = next_status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::submit(
      const std::shared_ptr<PrimitiveOrder>& order) {
    auto lock = std::lock_guard(m_mutex);
    if(order->get_info().m_fields.m_quantity <= 0) {
      enqueue(order, OrderStatus::REJECTED, order->get_info().m_timestamp, 0,
        Money::ZERO, "Invalid quantity.");
      return;
    } else if(order->get_info().m_fields.m_type == OrderType::MARKET &&
        (m_bbo.m_bid.m_price == Money::ZERO ||
          m_bbo.m_ask.m_price == Money::ZERO)) {
      enqueue(order, OrderStatus::REJECTED, order->get_info().m_timestamp, 0,
        Money::ZERO, "No BBO quote available.");
      return;
    }
    if(update_session(m_time_client->get_time())) {
      evaluate_held_orders();
    }
    expire_market_on_close_orders(m_time_client->get_time());
    if(m_is_closed && order->get_info().m_fields.m_time_in_force.get_type() ==
        TimeInForce::Type::MOC) {
      enqueue(order, OrderStatus::REJECTED, order->get_info().m_timestamp, 0,
        Money::ZERO, "Market closed.");
      return;
    }
    enqueue(
      order, OrderStatus::NEW, order->get_info().m_timestamp, 0, Money::ZERO);
    m_entries[order->get_info().m_id] = make_entry(
      *order, OrderStatus::NEW, order->get_info().m_fields.m_quantity);
    auto next_status = evaluate(order, OrderStatus::NEW);
    if(is_terminal(next_status)) {
      m_pegged_entries.erase(order->get_info().m_id);
      m_entries.erase(order->get_info().m_id);
      return;
    }
    m_orders.push_back(order);
    m_entries[order->get_info().m_id].m_status = next_status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::cancel(
      const std::shared_ptr<PrimitiveOrder>& order) {
    auto lock = std::lock_guard(m_mutex);
    auto i = m_entries.find(order->get_info().m_id);
    if(i == m_entries.end() || is_terminal(i->second.m_status)) {
      return;
    }
    auto timestamp = m_time_client->get_time();
    enqueue(order, OrderStatus::PENDING_CANCEL, timestamp, 0, Money::ZERO);
    enqueue(order, OrderStatus::CANCELED, timestamp, 0, Money::ZERO);
    i->second.m_status = OrderStatus::CANCELED;
    erase(order);
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::update(
      const std::shared_ptr<PrimitiveOrder>& order,
      const ExecutionReport& report) {
    auto lock = std::lock_guard(m_mutex);
    auto i = m_entries.find(order->get_info().m_id);
    if(i != m_entries.end()) {
      if(is_terminal(i->second.m_status)) {
        return;
      }
      i->second.m_status = report.m_status;
      i->second.m_remaining_quantity -= report.m_last_quantity;
      if(is_terminal(report.m_status)) {
        erase(order);
      }
    }
    auto timestamp = [&] {
      if(report.m_timestamp.is_special()) {
        return m_time_client->get_time();
      }
      return report.m_timestamp;
    }();
    m_reports->push([=, this] {
      order->with([&] (auto status, const auto& reports) {
        if(is_terminal(status) ||
            reports.empty() && report.m_status != OrderStatus::PENDING_NEW) {
          return;
        }
        auto updated_report = report;
        if(reports.empty()) {
          updated_report.m_sequence = 0;
        } else {
          updated_report.m_sequence = reports.back().m_sequence + 1;
        }
        updated_report.m_timestamp = timestamp;
        order->update(updated_report);
      });
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::update(const BboQuote& bbo) {
    auto has_expired = false;
    auto is_evaluating = false;
    {
      auto lock = std::lock_guard(m_mutex);
      if(get_session_date(bbo.m_timestamp) != m_date) {
        has_expired = expire_market_on_close_orders();
        set_session_timestamps(bbo.m_timestamp);
      }
      if(expire_market_on_close_orders(bbo.m_timestamp)) {
        has_expired = true;
      }
      auto has_opened = update_session(bbo.m_timestamp);
      auto is_price_unchanged = bbo.m_bid.m_price == m_bbo.m_bid.m_price &&
        bbo.m_ask.m_price == m_bbo.m_ask.m_price;
      m_bbo = bbo;
      if(m_phase == Phase::OPENING && m_cross_time.is_special() &&
          m_uncross_time.is_special() && !is_crossed()) {
        m_uncross_time = bbo.m_timestamp;
      }
      is_evaluating = !is_price_unchanged || has_opened;
    }
    if(has_expired) {
      m_reports->flush();
    }
    if(is_evaluating) {
      evaluate_orders();
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::evaluate_orders() {
    auto index = std::size_t(0);
    while(true) {
      auto has_update = false;
      {
        auto lock = std::lock_guard(m_mutex);
        if(index >= m_orders.size()) {
          break;
        }
        auto order = m_orders[index];
        auto& entry = m_entries[order->get_info().m_id];
        auto next_status = evaluate(order, entry.m_status);
        if(is_terminal(next_status)) {
          entry.m_status = next_status;
          erase(order);
          has_update = true;
        }
      }
      if(has_update) {
        m_reports->flush();
      } else {
        ++index;
      }
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::update(const TimeAndSale& time_and_sale) {
    auto is_closing = false;
    auto is_cross = false;
    auto has_opened = false;
    auto has_expired = false;
    {
      auto lock = std::lock_guard(m_mutex);
      if(get_session_date(time_and_sale.m_timestamp) != m_date) {
        has_expired = expire_market_on_close_orders();
        set_session_timestamps(time_and_sale.m_timestamp);
      }
      auto is_listing = from_market_center(time_and_sale.m_market_center,
        m_ticker.get_venue()).m_venue == m_ticker.get_venue();
      is_closing = is_listing && time_and_sale.m_condition.m_type ==
        TimeAndSale::Condition::Type::CLOSE;
      if(is_closing) {
        m_closing_cross_time = time_and_sale.m_timestamp;
      } else {
        if(expire_market_on_close_orders(time_and_sale.m_timestamp)) {
          has_expired = true;
        }
        is_cross = is_opening_cross(time_and_sale, is_listing);
      }
      if(!is_cross) {
        has_opened = update_session(time_and_sale.m_timestamp, is_listing);
      }
    }
    if(has_expired) {
      m_reports->flush();
    }
    if(is_closing || is_cross) {
      cross(time_and_sale, is_closing);
      return;
    }
    if(has_opened) {
      evaluate_orders();
    }
    match(time_and_sale);
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::update(const BookQuote& book_quote) {
    auto lock = std::lock_guard(m_mutex);
    auto delta = apply(book_quote);
    if(delta == 0) {
      return;
    }
    advance(Level(book_quote.m_venue, book_quote.m_quote.m_side,
      book_quote.m_quote.m_price), delta);
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::set_session_timestamps(
      boost::posix_time::ptime timestamp) {
    m_date = get_session_date(timestamp);
    m_session_timestamp = timestamp;
    m_listings.clear();
    m_levels.clear();
    m_sessions.clear();
    auto venue = m_ticker.get_venue();
    m_session = load_session(venue);
    m_venue_close_time = m_session.m_close;
    if(m_venue_close_time.is_special()) {
      m_venue_close_time = venue_to_utc(
        venue, boost::posix_time::ptime(m_date, boost::posix_time::hours(16)));
    }
    m_closing_cross_time = boost::posix_time::not_a_date_time;
    m_is_closed = timestamp >= m_venue_close_time + CLOSING_PERIOD;
    m_phase = get_phase(timestamp);
    m_cross_time = boost::posix_time::not_a_date_time;
    m_uncross_time = boost::posix_time::not_a_date_time;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  boost::gregorian::date TickerOrderSimulator<T>::get_session_date(
      boost::posix_time::ptime timestamp) const {
    return utc_to_venue(m_ticker.get_venue(), timestamp).date();
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  const typename TickerOrderSimulator<T>::Session&
      TickerOrderSimulator<T>::load_session(Venue venue) {
    auto i = m_sessions.find(venue);
    if(i != m_sessions.end()) {
      return i->second;
    }
    auto session = Session();
    for(auto& event : TRADING_SCHEDULE.find(m_session_timestamp, venue)) {
      if(event.m_code == PRE_OPEN_EVENT) {
        session.m_pre_open = event.m_timestamp;
      } else if(event.m_code == OPEN_EVENT) {
        session.m_open = event.m_timestamp;
      } else if(event.m_code == CLOSE_EVENT) {
        session.m_close = event.m_timestamp;
      }
    }
    return m_sessions.emplace(venue, session).first->second;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  typename TickerOrderSimulator<T>::Phase TickerOrderSimulator<T>::get_phase(
      boost::posix_time::ptime timestamp) const {
    if(m_session.m_open.is_special()) {
      return Phase::CONTINUOUS;
    }
    if(timestamp < m_session.m_open) {
      if(!m_session.m_pre_open.is_special() &&
          timestamp >= m_session.m_pre_open) {
        return Phase::PRE_OPEN;
      }
      return Phase::CONTINUOUS;
    }
    if(is_crossed() && timestamp < m_venue_close_time) {
      return Phase::OPENING;
    }
    return Phase::CONTINUOUS;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::is_crossed() const {
    return m_bbo.m_bid.m_price != Money::ZERO &&
      m_bbo.m_ask.m_price != Money::ZERO &&
      m_bbo.m_bid.m_price >= m_bbo.m_ask.m_price;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::is_held(const OrderEntry& entry) {
    if(entry.m_session_venue == m_ticker.get_venue()) {
      return m_phase != Phase::CONTINUOUS;
    }
    auto& session = load_session(entry.m_session_venue);
    if(session.m_pre_open.is_special() || session.m_open.is_special()) {
      return false;
    }
    auto timestamp = m_time_client->get_time();
    return timestamp >= session.m_pre_open && timestamp < session.m_open;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::update_session(
      boost::posix_time::ptime timestamp, bool is_listing_print) {
    if(m_phase == Phase::CONTINUOUS) {
      if(get_phase(timestamp) == Phase::PRE_OPEN) {
        m_phase = Phase::PRE_OPEN;
      }
      return false;
    }
    auto has_opened = [&] {
      if(m_phase == Phase::PRE_OPEN) {
        if(timestamp < m_session.m_open) {
          return false;
        }
        m_phase = Phase::OPENING;
        return !is_crossed();
      } else if(!m_cross_time.is_special()) {
        return timestamp > m_cross_time;
      }
      return is_listing_print && !m_uncross_time.is_special() &&
        timestamp > m_uncross_time;
    }();
    if(has_opened) {
      m_phase = Phase::CONTINUOUS;
    }
    return has_opened;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::is_opening_cross(
      const TimeAndSale& time_and_sale, bool is_listing) {
    if(!is_listing) {
      return false;
    }
    auto is_open =
      time_and_sale.m_condition.m_type == TimeAndSale::Condition::Type::OPEN;
    if(m_phase == Phase::CONTINUOUS) {
      return is_open;
    }
    if(!is_open) {
      if(time_and_sale.m_timestamp < m_session.m_open) {
        return false;
      }
      if(!m_cross_time.is_special()) {
        return time_and_sale.m_timestamp == m_cross_time;
      }
      if(m_uncross_time.is_special()) {
        if(!is_crossed()) {
          return false;
        }
      } else if(time_and_sale.m_timestamp > m_uncross_time) {
        return false;
      }
    }
    m_phase = Phase::OPENING;
    m_cross_time = time_and_sale.m_timestamp;
    return true;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::is_trade(
      const TimeAndSale::Condition& condition) const {
    if(condition.m_type == TimeAndSale::Condition::Type::NONE) {
      return condition.m_code == REGULAR_SALE;
    }
    return condition.m_type != TimeAndSale::Condition::Type::CORRECTION &&
      condition.m_type != TimeAndSale::Condition::Type::CANCELLATION;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::expire_market_on_close_orders() {
    auto has_update = false;
    auto index = std::size_t(0);
    while(index < m_orders.size()) {
      auto order = m_orders[index];
      if(order->get_info().m_fields.m_time_in_force.get_type() ==
          TimeInForce::Type::MOC) {
        enqueue(order, OrderStatus::EXPIRED, m_time_client->get_time(), 0,
          Money::ZERO);
        erase(order);
        has_update = true;
      } else {
        ++index;
      }
    }
    return has_update;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::expire_market_on_close_orders(
      boost::posix_time::ptime timestamp) {
    if(m_is_closed) {
      return false;
    }
    if(m_closing_cross_time.is_special()) {
      if(timestamp < m_venue_close_time + CLOSING_PERIOD) {
        return false;
      }
    } else if(timestamp <= m_closing_cross_time) {
      return false;
    }
    m_is_closed = true;
    return expire_market_on_close_orders();
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::evaluate_held_orders() {
    auto index = std::size_t(0);
    while(index < m_orders.size()) {
      auto order = m_orders[index];
      auto& entry = m_entries[order->get_info().m_id];
      auto next_status = evaluate(order, entry.m_status);
      if(is_terminal(next_status)) {
        entry.m_status = next_status;
        erase(order);
      } else {
        ++index;
      }
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::cross(
      const TimeAndSale& time_and_sale, bool is_closing) {
    auto size = time_and_sale.m_size - time_and_sale.m_size % BOARD_LOT;
    if(!is_trade(time_and_sale.m_condition) || size <= 0) {
      return;
    }
    auto has_update = false;
    {
      auto lock = std::lock_guard(m_mutex);
      if(cross(time_and_sale, Side::ASK, size, is_closing)) {
        has_update = true;
      }
      if(cross(time_and_sale, Side::BID, size, is_closing)) {
        has_update = true;
      }
    }
    if(has_update) {
      m_reports->flush();
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::cross(const TimeAndSale& time_and_sale,
      Side side, Quantity size, bool is_closing) {
    auto direction = get_direction(side);
    auto available = size;
    auto has_update = false;
    while(available > 0) {
      auto selection = std::shared_ptr<PrimitiveOrder>();
      for(auto& order : m_orders) {
        auto& fields = order->get_info().m_fields;
        auto is_market = fields.m_type == OrderType::MARKET;
        if(fields.m_side != side || !is_closing &&
            fields.m_time_in_force.get_type() == TimeInForce::Type::MOC ||
            !is_market && (fields.m_type != OrderType::LIMIT ||
              direction * time_and_sale.m_price > direction * fields.m_price)) {
          continue;
        }
        auto i = m_entries.find(order->get_info().m_id);
        if(i == m_entries.end() ||
            i->second.m_status == OrderStatus::PENDING_NEW ||
            is_terminal(i->second.m_status) ||
            i->second.m_session_venue != m_ticker.get_venue()) {
          continue;
        }
        if(selection) {
          auto& selected_fields = selection->get_info().m_fields;
          if(selected_fields.m_type == OrderType::MARKET || !is_market &&
              direction * selected_fields.m_price >=
                direction * fields.m_price) {
            continue;
          }
        }
        selection = order;
      }
      if(!selection) {
        break;
      }
      auto& entry = m_entries[selection->get_info().m_id];
      auto quantity = std::min(entry.m_remaining_quantity, available);
      entry.m_status = fill(selection, time_and_sale.m_price, quantity);
      if(is_terminal(entry.m_status)) {
        erase(selection);
      }
      available -= quantity;
      has_update = true;
    }
    return has_update;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::submit_pegged(const PrimitiveOrder& order) {
    auto& fields = order.get_info().m_fields;
    auto entry = PeggedOrderEntry();
    entry.m_exec_inst = PRIMARY_PEG;
    if(auto tag = find_field(fields, EXEC_INST_KEY)) {
      if(auto* value = std::get_if<std::string>(&tag->get_value())) {
        auto stream = std::istringstream(*value);
        auto token = std::string();
        while(stream >> token) {
          if(token == PRIMARY_PEG || token == MARKET_PEG ||
              token == MID_PRICE_PEG) {
            entry.m_exec_inst = token;
            break;
          }
        }
      }
    }
    if(auto tag = find_field(fields, PEG_DIFFERENCE_KEY)) {
      if(auto* money = std::get_if<Money>(&tag->get_value())) {
        entry.m_peg_difference = *money;
      }
    }
    auto direction = get_direction(fields.m_side);
    auto [same_price, opposite_price] = pick(fields.m_side,
      std::pair(m_bbo.m_ask.m_price, m_bbo.m_bid.m_price),
      std::pair(m_bbo.m_bid.m_price, m_bbo.m_ask.m_price));
    entry.m_effective_price = [&] {
      if(entry.m_exec_inst == MARKET_PEG) {
        return opposite_price;
      } else if(entry.m_exec_inst == MID_PRICE_PEG) {
        return (same_price + opposite_price) / 2;
      }
      return same_price;
    }();
    entry.m_effective_price -= direction * entry.m_peg_difference;
    if(fields.m_price != Money::ZERO &&
        direction * entry.m_effective_price > direction * fields.m_price) {
      entry.m_effective_price = fields.m_price;
    }
    m_pegged_entries[order.get_info().m_id] = entry;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::enqueue(
      const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status,
      boost::posix_time::ptime timestamp, Quantity last_quantity,
      Money last_price, std::string text) {
    m_reports->push([=] {
      order->with([&] (auto current_status, const auto& reports) {
        if(reports.empty() || is_terminal(current_status)) {
          return;
        }
        auto updated_report = make_update(reports.back(), status, timestamp);
        updated_report.m_last_quantity = last_quantity;
        updated_report.m_last_price = last_price;
        updated_report.m_text = text;
        order->update(updated_report);
      });
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::enqueue(
      const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status,
      boost::posix_time::ptime timestamp, Quantity last_quantity,
      Money last_price) {
    enqueue(order, status, timestamp, last_quantity, last_price, "");
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus TickerOrderSimulator<T>::fill(
      const std::shared_ptr<PrimitiveOrder>& order, Money price,
      Quantity quantity) {
    auto& remaining = m_entries[order->get_info().m_id].m_remaining_quantity;
    remaining -= quantity;
    auto status = [&] {
      if(remaining <= 0) {
        return OrderStatus::FILLED;
      }
      return OrderStatus::PARTIALLY_FILLED;
    }();
    enqueue(order, status, m_time_client->get_time(), quantity, price);
    return status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus TickerOrderSimulator<T>::fill(
      const std::shared_ptr<PrimitiveOrder>& order, Money price) {
    return fill(
      order, price, m_entries[order->get_info().m_id].m_remaining_quantity);
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus TickerOrderSimulator<T>::evaluate(
      const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status) {
    auto& fields = order->get_info().m_fields;
    if(status == OrderStatus::PENDING_NEW || is_terminal(status) ||
        fields.m_time_in_force.get_type() == TimeInForce::Type::MOC) {
      return status;
    }
    if(m_bbo.m_bid.m_price == Money::ZERO ||
        m_bbo.m_ask.m_price == Money::ZERO) {
      return status;
    }
    if(m_phase != Phase::CONTINUOUS ||
        is_held(m_entries[order->get_info().m_id])) {
      return status;
    }
    if(fields.m_type == OrderType::MARKET) {
      auto price =
        pick(fields.m_side, m_bbo.m_bid.m_price, m_bbo.m_ask.m_price);
      return fill(order, price);
    } else if(fields.m_type == OrderType::PEGGED) {
      if(!m_pegged_entries.contains(order->get_info().m_id)) {
        submit_pegged(*order);
      }
      return update_pegged(order, status);
    } else if(fields.m_side == Side::BID &&
        m_bbo.m_ask.m_price <= fields.m_price) {
      return fill(order, m_bbo.m_ask.m_price);
    } else if(fields.m_side == Side::ASK &&
        m_bbo.m_bid.m_price >= fields.m_price) {
      return fill(order, m_bbo.m_bid.m_price);
    }
    return status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus TickerOrderSimulator<T>::update_pegged(
      const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status) {
    auto i = m_pegged_entries.find(order->get_info().m_id);
    if(i == m_pegged_entries.end()) {
      return status;
    }
    auto& entry = i->second;
    auto side = order->get_info().m_fields.m_side;
    auto direction = get_direction(side);
    auto [same_price, opposite_price] =
      pick(side, std::pair(m_bbo.m_ask.m_price, m_bbo.m_bid.m_price),
        std::pair(m_bbo.m_bid.m_price, m_bbo.m_ask.m_price));
    auto candidate = [&] {
      if(entry.m_exec_inst == MARKET_PEG) {
        return opposite_price;
      } else if(entry.m_exec_inst == MID_PRICE_PEG) {
        return (same_price + opposite_price) / 2;
      }
      return same_price;
    }();
    candidate -= direction * entry.m_peg_difference;
    if(direction * candidate > direction * entry.m_effective_price) {
      entry.m_effective_price = candidate;
    }
    auto limit_price = order->get_info().m_fields.m_price;
    if(limit_price != Money::ZERO &&
        direction * entry.m_effective_price > direction * limit_price) {
      entry.m_effective_price = limit_price;
    }
    if(direction * opposite_price <= direction * entry.m_effective_price) {
      m_pegged_entries.erase(i);
      return fill(order, opposite_price);
    }
    return status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  typename TickerOrderSimulator<T>::OrderEntry
      TickerOrderSimulator<T>::make_entry(const PrimitiveOrder& order,
        OrderStatus status, Quantity remaining_quantity) {
    auto entry = OrderEntry(status, remaining_quantity, Venue(), 0, 0, Venue());
    auto& fields = order.get_info().m_fields;
    entry.m_session_venue =
      Details::get_posting_venue(fields.m_destination, m_ticker.get_venue());
    if(!entry.m_session_venue) {
      entry.m_session_venue = m_ticker.get_venue();
    }
    if(fields.m_type != OrderType::LIMIT ||
        fields.m_time_in_force.get_type() == TimeInForce::Type::MOC) {
      return entry;
    }
    entry.m_venue =
      Details::get_posting_venue(fields.m_destination, m_ticker.get_venue());
    if(entry.m_venue) {
      auto i =
        m_levels.find(Level(entry.m_venue, fields.m_side, fields.m_price));
      if(i != m_levels.end()) {
        entry.m_queue_quantity = i->second;
      }
    }
    return entry;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  Quantity TickerOrderSimulator<T>::apply(const BookQuote& book_quote) {
    auto listing = Listing(book_quote.m_venue, book_quote.m_quote.m_side,
      book_quote.m_quote.m_price, book_quote.m_mpid);
    auto previous = [&] {
      auto i = m_listings.find(listing);
      if(i == m_listings.end()) {
        return Quantity(0);
      }
      return i->second;
    }();
    auto quantity = std::max(Quantity(0), book_quote.m_quote.m_size);
    auto delta = quantity - previous;
    if(delta == 0) {
      return 0;
    }
    if(quantity <= 0) {
      m_listings.erase(listing);
    } else {
      m_listings[listing] = quantity;
    }
    auto level = Level(book_quote.m_venue, book_quote.m_quote.m_side,
      book_quote.m_quote.m_price);
    auto size = m_levels[level] + delta;
    if(size <= 0) {
      m_levels.erase(level);
    } else {
      m_levels[level] = size;
    }
    return delta;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::advance(const Level& level, Quantity delta) {
    for(auto& order : m_orders) {
      auto i = m_entries.find(order->get_info().m_id);
      if(i == m_entries.end() || !i->second.m_venue) {
        continue;
      }
      auto& fields = order->get_info().m_fields;
      if(Level(i->second.m_venue, fields.m_side, fields.m_price) != level) {
        continue;
      }
      auto& entry = i->second;
      if(delta > 0) {
        entry.m_trailing_quantity += delta;
      } else {
        auto remainder = -delta;
        auto consumed = std::min(entry.m_trailing_quantity, remainder);
        entry.m_trailing_quantity -= consumed;
        remainder -= consumed;
        entry.m_queue_quantity -= std::min(entry.m_queue_quantity, remainder);
      }
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  Quantity TickerOrderSimulator<T>::allocate(
      const std::shared_ptr<PrimitiveOrder>& order, Quantity available) {
    auto& entry = m_entries[order->get_info().m_id];
    auto quantity = std::min(entry.m_remaining_quantity, available);
    auto status = fill(order, order->get_info().m_fields.m_price, quantity);
    entry.m_status = status;
    if(is_terminal(status)) {
      erase(order);
    }
    return quantity;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  bool TickerOrderSimulator<T>::match(const TimeAndSale& time_and_sale,
      Side side, Quantity size) {
    auto direction = get_direction(side);
    auto available = size;
    auto has_update = false;
    while(available > 0) {
      auto selection = std::shared_ptr<PrimitiveOrder>();
      for(auto& order : m_orders) {
        auto& fields = order->get_info().m_fields;
        if(fields.m_side != side || fields.m_type != OrderType::LIMIT ||
            fields.m_time_in_force.get_type() == TimeInForce::Type::MOC ||
            direction * time_and_sale.m_price >= direction * fields.m_price) {
          continue;
        }
        auto i = m_entries.find(order->get_info().m_id);
        if(i == m_entries.end() ||
            i->second.m_status == OrderStatus::PENDING_NEW ||
            is_terminal(i->second.m_status) || is_held(i->second)) {
          continue;
        }
        if(selection && direction * selection->get_info().m_fields.m_price >=
            direction * fields.m_price) {
          continue;
        }
        selection = order;
      }
      if(!selection) {
        break;
      }
      available -= allocate(selection, available);
      has_update = true;
    }
    while(available > 0) {
      auto selection = std::shared_ptr<PrimitiveOrder>();
      for(auto& order : m_orders) {
        auto& fields = order->get_info().m_fields;
        if(fields.m_side != side || fields.m_type != OrderType::LIMIT ||
            fields.m_time_in_force.get_type() == TimeInForce::Type::MOC ||
            time_and_sale.m_price != fields.m_price) {
          continue;
        }
        auto i = m_entries.find(order->get_info().m_id);
        if(i == m_entries.end() ||
            i->second.m_status == OrderStatus::PENDING_NEW ||
            is_terminal(i->second.m_status) || !i->second.m_venue ||
            i->second.m_queue_quantity > 0 || is_held(i->second)) {
          continue;
        }
        if(VENUES.from(i->second.m_venue).m_market_center !=
            time_and_sale.m_market_center) {
          continue;
        }
        selection = order;
        break;
      }
      if(!selection) {
        break;
      }
      available -= allocate(selection, available);
      has_update = true;
    }
    return has_update;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::match(const TimeAndSale& time_and_sale) {
    auto size = time_and_sale.m_size - time_and_sale.m_size % BOARD_LOT;
    if(!is_trade(time_and_sale.m_condition) || size <= 0) {
      return;
    }
    auto has_update = false;
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_orders.empty()) {
        return;
      }
      if(match(time_and_sale, Side::ASK, size)) {
        has_update = true;
      }
      if(match(time_and_sale, Side::BID, size)) {
        has_update = true;
      }
    }
    if(has_update) {
      m_reports->flush();
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::erase(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_pegged_entries.erase(order->get_info().m_id);
    m_entries.erase(order->get_info().m_id);
    std::erase(m_orders, order);
  }
}

#endif
