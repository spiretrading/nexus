#ifndef ASX_TRADE_ITCH_MARKET_DATA_FEED_CLIENT_HPP
#define ASX_TRADE_ITCH_MARKET_DATA_FEED_CLIENT_HPP
#include <atomic>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <syncstream>
#include <unordered_set>
#include <Beam/Utilities/Expect.hpp>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchClient.hpp"
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchConfiguration.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/OrderToBboQuoteModel.hpp"

namespace Nexus {

  /**
   * Publishes ASX market data and the venue's best bid and ask.
   * @tparam M The client publishing market data.
   * @tparam C The client delivering ordered ITCH messages.
   */
  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  class AsxTradeItchMarketDataFeedClient {
    public:

      /** The client publishing market data. */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The client delivering ordered ITCH messages. */
      using Client = Beam::dereference_t<C>;

      /**
       * Constructs an AsxTradeItchMarketDataFeedClient.
       * @param config The partition's configuration.
       * @param feed_client Initializes the market data publisher.
       * @param client Initializes the ordered message source.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<C> CF>
      AsxTradeItchMarketDataFeedClient(
        AsxTradeItchConfiguration config, MF&& feed_client, CF&& client);

      ~AsxTradeItchMarketDataFeedClient();

      /** Returns whether message reception has finished. */
      bool is_finished() const;

      /** Returns the exception that stopped message reception, if any. */
      std::exception_ptr get_exception() const;

      void close();

    private:
      struct OrderEntry {
        std::string m_mpid;
        std::int32_t m_price;
        Quantity m_quantity;
        Side m_side;
      };
      struct OrderAdapter {
        using OrderId = std::uint64_t;
        using Order = OrderEntry;
        double m_price_scale = 1;
        Venue m_venue;

        Money get_price(std::int32_t price) const;
        BookQuote make_quote(
          const Order& order, boost::posix_time::ptime timestamp) const;
      };
      using Model = OrderToBboQuoteModel<OrderAdapter>;
      struct Book {
        Ticker m_ticker;
        std::string m_state;
        Model m_model;
      };
      AsxTradeItchConfiguration m_config;
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<C> m_client;
      std::unordered_map<std::uint32_t, Book> m_books;
      std::unordered_set<std::uint32_t> m_excluded_books;
      boost::posix_time::ptime m_seconds;
      Beam::Sync<std::exception_ptr> m_exception;
      std::atomic_bool m_is_finished;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      static void log(const AsxTradeItchMessage& message);
      static void log_raw(const AsxTradeItchMessage& message);
      static Money get_price(const Book& book, std::int32_t price);
      AsxTradeItchMarketDataFeedClient(
        const AsxTradeItchMarketDataFeedClient&) = delete;
      AsxTradeItchMarketDataFeedClient& operator =(
        const AsxTradeItchMarketDataFeedClient&) = delete;
      boost::posix_time::ptime get_timestamp(std::uint32_t nanoseconds) const;
      Book* find_book(std::uint32_t id);
      void publish(const Book& book, const Model::Update& update);
      void publish(const Book& book, Side side, Money price,
        std::uint64_t quantity, const std::string& owner,
        const std::string& counterparty, bool is_cross,
        boost::posix_time::ptime timestamp);
      void clear(Book& book, boost::posix_time::ptime timestamp);
      template<typename D> requires
        std::same_as<D, AsxTradeItchOrderBookDirectory> ||
          std::same_as<D, AsxTradeItchCombinationOrderBookDirectory>
      void add(const D& message);
      template<typename A> requires
        std::same_as<A, AsxTradeItchAddOrder> ||
          std::same_as<A, AsxTradeItchAddOrderWithParticipant>
      void add(const A& message);
      template<typename E> requires
        std::same_as<E, AsxTradeItchOrderExecuted> ||
          std::same_as<E, AsxTradeItchOrderExecutedAtPrice>
      void execute(const E& message);
      void replace(const AsxTradeItchOrderReplace& message);
      void remove(const AsxTradeItchOrderDelete& message);
      void report(const AsxTradeItchTrade& message);
      void report(const AsxTradeItchEquilibriumPriceUpdate& message);
      void dispatch(const AsxTradeItchMessage& message);
      void read_loop();
  };

  template<typename M, typename C>
  AsxTradeItchMarketDataFeedClient(AsxTradeItchConfiguration, M&&, C&&) ->
    AsxTradeItchMarketDataFeedClient<
      std::remove_cvref_t<M>, std::remove_cvref_t<C>>;

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  template<Beam::Initializes<M> MF, Beam::Initializes<C> CF>
  AsxTradeItchMarketDataFeedClient<M, C>::AsxTradeItchMarketDataFeedClient(
      AsxTradeItchConfiguration config, MF&& feed_client, CF&& client)
      : m_config(std::move(config)),
        m_feed_client(std::forward<MF>(feed_client)),
        m_client(std::forward<CF>(client)),
        m_is_finished(false) {
    m_read_loop = Beam::spawn(std::bind_front(
      &AsxTradeItchMarketDataFeedClient::read_loop, this));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  AsxTradeItchMarketDataFeedClient<M, C>::~AsxTradeItchMarketDataFeedClient() {
    close();
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  bool AsxTradeItchMarketDataFeedClient<M, C>::is_finished() const {
    return m_is_finished;
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  std::exception_ptr
      AsxTradeItchMarketDataFeedClient<M, C>::get_exception() const {
    return m_exception.load();
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  Money AsxTradeItchMarketDataFeedClient<M, C>::OrderAdapter::get_price(
      std::int32_t price) const {
    return Money(Quantity::from_representation(
      price * static_cast<Quantity>(Money::CENT).get_representation() /
        m_price_scale));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  BookQuote AsxTradeItchMarketDataFeedClient<M, C>::OrderAdapter::make_quote(
      const Order& order, boost::posix_time::ptime timestamp) const {
    return BookQuote(order.m_mpid, false, m_venue,
      Quote(get_price(order.m_price), order.m_quantity, order.m_side),
      timestamp);
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::log(
      const AsxTradeItchMessage& message) {
    visit(message, [] (const auto& message) {
      using Message = std::remove_cvref_t<decltype(message)>;
      if constexpr(std::same_as<Message, AsxTradeItchMessage>) {
        log_raw(message);
      } else {
        std::osyncstream(std::cout) << message << '\n';
      }
    });
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::log_raw(
      const AsxTradeItchMessage& message) {
    auto out = std::osyncstream(std::cout);
    out << "(message " << static_cast<char>(message.m_type) << ' ' <<
      std::hex << std::setfill('0');
    for(auto i = std::size_t(0);
        i != message.m_length - AsxTradeItchMessage::HEADER_LENGTH; ++i) {
      out << std::setw(2) << static_cast<unsigned int>(
        static_cast<unsigned char>(message.m_payload[i]));
    }
    out << ")\n";
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  Money AsxTradeItchMarketDataFeedClient<M, C>::get_price(
      const Book& book, std::int32_t price) {
    return book.m_model.get_adapter().get_price(price);
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  boost::posix_time::ptime
      AsxTradeItchMarketDataFeedClient<M, C>::get_timestamp(
        std::uint32_t nanoseconds) const {
    if(m_seconds.is_not_a_date_time()) {
      boost::throw_with_location(Beam::IOException(
        "ITCH timestamp is unavailable. Start with a Glimpse snapshot."));
    }
    return m_seconds + boost::posix_time::microseconds(nanoseconds / 1000);
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  typename AsxTradeItchMarketDataFeedClient<M, C>::Book*
      AsxTradeItchMarketDataFeedClient<M, C>::find_book(std::uint32_t id) {
    auto book = m_books.find(id);
    if(book == m_books.end()) {
      if(m_excluded_books.contains(id)) {
        return nullptr;
      }
      boost::throw_with_location(Beam::IOException(
        "Unknown ITCH book " + std::to_string(id) +
        ". Start with a Glimpse snapshot."));
    }
    return &book->second;
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::publish(
      const Book& book, const Model::Update& update) {
    for(auto& quote : update.m_quotes) {
      m_feed_client->publish(TickerBookQuote(quote, book.m_ticker));
    }
    if(update.m_is_bbo_changed) {
      m_feed_client->publish(
        TickerBboQuote(book.m_model.get_bbo(), book.m_ticker));
    }
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::publish(const Book& book,
      Side side, Money price, std::uint64_t quantity, const std::string& owner,
      const std::string& counterparty, bool is_cross,
      boost::posix_time::ptime timestamp) {
    if(quantity == 0) {
      return;
    }
    auto condition = TimeAndSale::Condition();
    if(!is_cross) {
      condition.m_type = TimeAndSale::Condition::Type::REGULAR;
      condition.m_code = "@";
    } else if(book.m_state == "CSPA" || book.m_state == "PRE_CSPA") {
      condition.m_type = TimeAndSale::Condition::Type::CLOSE;
      condition.m_code = "C";
    } else if(book.m_state == "PRE_OPEN" || book.m_state == "OSPA") {
      condition.m_type = TimeAndSale::Condition::Type::OPEN;
      condition.m_code = "O";
    } else {
      condition.m_code = "AUCTION";
    }
    auto buyer = std::string();
    auto seller = std::string();
    if(side == Side::BID) {
      buyer = owner;
      seller = counterparty;
    } else if(side == Side::ASK) {
      buyer = counterparty;
      seller = owner;
    }
    if(buyer.empty()) {
      buyer = "AU000";
    }
    if(seller.empty()) {
      seller = "AU000";
    }
    m_feed_client->publish(TickerTimeAndSale(
      TimeAndSale(timestamp, price, quantity, std::move(condition),
        VENUES.from(m_config.m_disseminating_venue).m_display_name,
        std::move(buyer), std::move(seller)), book.m_ticker));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::clear(
      Book& book, boost::posix_time::ptime timestamp) {
    publish(book, book.m_model.clear(timestamp));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  template<typename D> requires
    std::same_as<D, AsxTradeItchOrderBookDirectory> ||
      std::same_as<D, AsxTradeItchCombinationOrderBookDirectory>
  void AsxTradeItchMarketDataFeedClient<M, C>::add(const D& message) {
    auto timestamp = get_timestamp(message.m_nanoseconds);
    static constexpr auto EQUITY = std::uint8_t(5);
    if(message.m_financial_product != EQUITY) {
      auto i = m_books.find(message.m_order_book_id);
      if(i != m_books.end()) {
        clear(i->second, timestamp);
        m_books.erase(i);
      }
      m_excluded_books.insert(message.m_order_book_id);
      return;
    }
    m_excluded_books.erase(message.m_order_book_id);
    auto scale = std::pow(10.0, message.m_price_decimals);
    if(!std::isfinite(scale)) {
      boost::throw_with_location(
        Beam::IOException("Invalid ITCH price scale."));
    }
    auto ticker = Ticker(message.m_symbol, m_config.m_primary_venue);
    auto [entry, is_inserted] = m_books.try_emplace(message.m_order_book_id);
    auto& book = entry->second;
    if(!is_inserted && book.m_ticker != ticker) {
      clear(book, timestamp);
      book.m_state.clear();
    }
    auto is_repriced =
      is_inserted || book.m_model.get_adapter().m_price_scale != scale;
    book.m_ticker = ticker;
    m_feed_client->add(TickerInfo(
      ticker, message.m_long_name, std::string(), message.m_round_lot_size));
    if(is_repriced) {
      publish(book, book.m_model.set_adapter(
        OrderAdapter(scale, m_config.m_disseminating_venue), timestamp));
    }
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  template<typename A> requires
    std::same_as<A, AsxTradeItchAddOrder> ||
      std::same_as<A, AsxTradeItchAddOrderWithParticipant>
  void AsxTradeItchMarketDataFeedClient<M, C>::add(const A& message) {
    auto timestamp = get_timestamp(message.m_nanoseconds);
    auto entry = find_book(message.m_order_book_id);
    if(!entry) {
      return;
    }
    auto& book = *entry;
    auto mpid = [&] {
      if constexpr(std::same_as<A, AsxTradeItchAddOrderWithParticipant>) {
        if(!message.m_participant_id.empty()) {
          return message.m_participant_id;
        }
      }
      return std::string("AU000");
    }();
    publish(book, book.m_model.add(message.m_side, message.m_order_id,
      OrderEntry(std::move(mpid), message.m_price, message.m_quantity,
        message.m_side), timestamp));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  template<typename E> requires
    std::same_as<E, AsxTradeItchOrderExecuted> ||
      std::same_as<E, AsxTradeItchOrderExecutedAtPrice>
  void AsxTradeItchMarketDataFeedClient<M, C>::execute(const E& message) {
    auto timestamp = get_timestamp(message.m_nanoseconds);
    auto entry = find_book(message.m_order_book_id);
    if(!entry) {
      return;
    }
    auto& book = *entry;
    auto order =
      book.m_model.get_book(message.m_side).find_order(message.m_order_id);
    auto price = Money();
    if constexpr(std::same_as<E, AsxTradeItchOrderExecutedAtPrice>) {
      price = get_price(book, message.m_price);
    } else {
      if(!order) {
        return;
      }
      price = get_price(book, order->m_price);
    }
    if(order && order->m_quantity != 0 && message.m_executed_quantity != 0) {
      if(Quantity(message.m_executed_quantity) >= order->m_quantity) {
        publish(book, book.m_model.remove(
          message.m_side, message.m_order_id, timestamp));
      } else {
        publish(book, book.m_model.update(message.m_side, message.m_order_id,
          [&] (auto& order) {
            order.m_quantity -= Quantity(message.m_executed_quantity);
          }, timestamp));
      }
    }
    auto is_cross = false;
    if constexpr(std::same_as<E, AsxTradeItchOrderExecutedAtPrice>) {
      if(message.m_printable != 'Y') {
        return;
      }
      is_cross = message.m_occurred_at_cross == 'Y';
    }
    publish(book, message.m_side, price, message.m_executed_quantity,
      message.m_owner, message.m_counterparty, is_cross, timestamp);
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::replace(
      const AsxTradeItchOrderReplace& message) {
    auto timestamp = get_timestamp(message.m_nanoseconds);
    auto entry = find_book(message.m_order_book_id);
    if(!entry) {
      return;
    }
    auto& book = *entry;
    publish(book, book.m_model.update(message.m_side, message.m_order_id,
      [&] (auto& order) {
        order.m_price = message.m_price;
        order.m_quantity = message.m_quantity;
      }, timestamp));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::remove(
      const AsxTradeItchOrderDelete& message) {
    auto timestamp = get_timestamp(message.m_nanoseconds);
    auto entry = find_book(message.m_order_book_id);
    if(!entry) {
      return;
    }
    auto& book = *entry;
    publish(
      book, book.m_model.remove(message.m_side, message.m_order_id, timestamp));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::report(
      const AsxTradeItchTrade& message) {
    if(message.m_printable != 'Y') {
      return;
    }
    auto timestamp = get_timestamp(message.m_nanoseconds);
    auto entry = find_book(message.m_order_book_id);
    if(!entry) {
      return;
    }
    auto& book = *entry;
    publish(book, message.m_side, get_price(book, message.m_price),
      message.m_quantity, message.m_owner, message.m_counterparty,
      message.m_occurred_at_cross == 'Y', timestamp);
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::report(
      const AsxTradeItchEquilibriumPriceUpdate& message) {
    auto timestamp = get_timestamp(message.m_nanoseconds);
    auto entry = find_book(message.m_order_book_id);
    if(!entry) {
      return;
    }
    auto& book = *entry;
    auto side = Side(Side::NONE);
    auto size = std::uint64_t(0);
    if(message.m_bid_quantity > message.m_ask_quantity) {
      side = Side::BID;
      size = message.m_bid_quantity - message.m_ask_quantity;
    } else if(message.m_ask_quantity > message.m_bid_quantity) {
      side = Side::ASK;
      size = message.m_ask_quantity - message.m_bid_quantity;
    }
    auto price = Money();
    static constexpr auto NO_PRICE = std::numeric_limits<std::int32_t>::min();
    if(message.m_equilibrium_price != NO_PRICE) {
      price = get_price(book, message.m_equilibrium_price);
    }
    m_feed_client->publish(VenueOrderImbalance(
      OrderImbalance(book.m_ticker, side, size, price, timestamp),
      m_config.m_disseminating_venue));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::dispatch(
      const AsxTradeItchMessage& message) {
    visit(message,
      [&] (const AsxTradeItchSeconds& message) {
        static const auto EPOCH =
          boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));
        m_seconds = EPOCH + boost::posix_time::seconds(message.m_seconds);
      },
      [&] (const AsxTradeItchOrderBookDirectory& message) { add(message); },
      [&] (const AsxTradeItchCombinationOrderBookDirectory& message) {
        add(message);
      },
      [&] (const AsxTradeItchAddOrder& message) { add(message); },
      [&] (const AsxTradeItchAddOrderWithParticipant& message) {
        add(message);
      },
      [&] (const AsxTradeItchOrderExecuted& message) { execute(message); },
      [&] (const AsxTradeItchOrderExecutedAtPrice& message) {
        execute(message);
      },
      [&] (const AsxTradeItchOrderReplace& message) { replace(message); },
      [&] (const AsxTradeItchOrderDelete& message) { remove(message); },
      [&] (const AsxTradeItchTrade& message) { report(message); },
      [&] (const AsxTradeItchEquilibriumPriceUpdate& message) {
        report(message);
      },
      [&] (const AsxTradeItchOrderBookState& message) {
        if(auto book = find_book(message.m_order_book_id)) {
          book->m_state = message.m_state;
        }
      },
      [&] (const AsxTradeItchSystemEvent& message) {
        if(message.m_event_code == 'O') {
          auto timestamp = get_timestamp(message.m_nanoseconds);
          for(auto& [id, book] : m_books) {
            clear(book, timestamp);
          }
          m_books.clear();
          m_excluded_books.clear();
        }
      });
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsAsxTradeItchClient<Beam::dereference_t<C>>
  void AsxTradeItchMarketDataFeedClient<M, C>::read_loop() {
    try {
      while(m_open_state.is_open()) {
        auto message = AsxTradeItchMessage();
        try {
          message = m_client->read();
        } catch(const Beam::EndOfFileException&) {
          break;
        }
        if(!m_open_state.is_open()) {
          break;
        }
        try {
          if(m_config.m_is_logging_messages) {
            log(message);
          }
          dispatch(message);
        } catch(const std::exception&) {
          log_raw(message);
          std::cout << std::flush;
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
