#ifndef OTC_LINK_MARKET_DATA_FEED_CLIENT_HPP
#define OTC_LINK_MARKET_DATA_FEED_CLIENT_HPP
#include <atomic>
#include <iomanip>
#include <iostream>
#include <syncstream>
#include <unordered_map>
#include <unordered_set>
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkMessages.hpp"

namespace Nexus {

  /**
   * Publishes OTC Link participant books and the OTC Link-only inside.
   * @tparam M The client publishing market data.
   * @tparam C The client delivering an ordered OTC Link-only channel.
   * @tparam T The time client used when clearing a reset channel.
   */
  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  class OtcLinkMarketDataFeedClient {
    public:

      /** The client publishing market data. */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The client delivering ordered OTC Link messages. */
      using OtcClient = Beam::dereference_t<C>;

      /** The source of the current time. */
      using TimeClient = Beam::dereference_t<T>;

      /**
       * Constructs an OtcLinkMarketDataFeedClient.
       * @param feed_client Initializes the market data publisher.
       * @param otc_client Initializes the ordered message source.
       * @param time_client Initializes the time source.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<C> CF,
        Beam::Initializes<T> TF>
      OtcLinkMarketDataFeedClient(
        MF&& feed_client, CF&& otc_client, TF&& time_client);

      ~OtcLinkMarketDataFeedClient();

      /** Returns whether message reception has finished. */
      bool is_finished() const;

      /** Returns the exception that stopped message reception, if any. */
      std::exception_ptr get_exception() const;

      /** Closes the clients and interrupts message reception. */
      void close();

    private:
      struct Entry {
        std::uint32_t m_security = 0;
        BboQuote m_quote;
        std::uint8_t m_flags = 0;
      };
      struct Participant : Entry {
        std::string m_mpid;
        BboQuote m_published;
      };
      struct Book {
        Ticker m_ticker;
        std::unordered_set<std::uint32_t> m_insides;
        BboQuote m_published;
      };
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<C> m_otc_client;
      Beam::local_ptr_t<T> m_time_client;
      std::unordered_map<std::uint32_t, Book> m_books;
      std::unordered_map<std::uint32_t, Participant> m_quotes;
      std::unordered_map<std::uint32_t, Entry> m_insides;
      std::uint64_t m_session;
      bool m_is_market_open;
      Beam::Sync<std::exception_ptr> m_exception;
      std::atomic_bool m_is_finished;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      static Venue get_venue(OtcLinkTier tier);
      static boost::posix_time::ptime get_timestamp(std::uint64_t timestamp);
      static Quantity convert(std::uint64_t value, std::uint64_t scale);
      static void log(
        const OtcLinkMessage& message, const std::exception& exception);
      OtcLinkMarketDataFeedClient(const OtcLinkMarketDataFeedClient&) = delete;
      OtcLinkMarketDataFeedClient& operator =(
        const OtcLinkMarketDataFeedClient&) = delete;
      Quote get_quote(const Entry& entry, Side side) const;
      void publish(std::uint32_t id, Participant& entry,
        boost::posix_time::ptime timestamp);
      void publish(Book& book, boost::posix_time::ptime timestamp);
      void withdraw(std::uint32_t id, Participant& entry,
        boost::posix_time::ptime timestamp);
      void clear(std::uint32_t security, boost::posix_time::ptime timestamp);
      void clear(boost::posix_time::ptime timestamp);
      void clear_quotes(boost::posix_time::ptime timestamp);
      void set_market_open(bool is_open, boost::posix_time::ptime timestamp);
      void process(const OtcLinkSecurity& message);
      template<typename Q> requires std::same_as<Q, OtcLinkQuote> ||
        std::same_as<Q, OtcLinkFractionalQuote>
      void process(const Q& message);
      template<typename Q> requires std::same_as<Q, OtcLinkQuoteUpdate> ||
        std::same_as<Q, OtcLinkFractionalQuoteUpdate>
      void process(const Q& message);
      template<typename Q> requires std::same_as<Q, OtcLinkInside> ||
        std::same_as<Q, OtcLinkFractionalInside>
      void process(const Q& message);
      template<typename Q> requires std::same_as<Q, OtcLinkInsideUpdate> ||
        std::same_as<Q, OtcLinkFractionalInsideUpdate>
      void process(const Q& message);
      void dispatch(const OtcLinkMessage& message);
      void read_loop();
  };

  template<typename M, typename C, typename T>
  OtcLinkMarketDataFeedClient(M&&, C&&, T&&) -> OtcLinkMarketDataFeedClient<
    std::remove_cvref_t<M>, std::remove_cvref_t<C>, std::remove_cvref_t<T>>;

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  template<Beam::Initializes<M> MF, Beam::Initializes<C> CF,
    Beam::Initializes<T> TF>
  OtcLinkMarketDataFeedClient<M, C, T>::OtcLinkMarketDataFeedClient(
      MF&& feed_client, CF&& otc_client, TF&& time_client)
      : m_feed_client(std::forward<MF>(feed_client)),
        m_otc_client(std::forward<CF>(otc_client)),
        m_time_client(std::forward<TF>(time_client)),
        m_session(0),
        m_is_market_open(true),
        m_is_finished(false) {
    m_read_loop = Beam::spawn(
      std::bind_front(&OtcLinkMarketDataFeedClient::read_loop, this));
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  OtcLinkMarketDataFeedClient<M, C, T>::~OtcLinkMarketDataFeedClient() {
    close();
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  bool OtcLinkMarketDataFeedClient<M, C, T>::is_finished() const {
    return m_is_finished;
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  std::exception_ptr
      OtcLinkMarketDataFeedClient<M, C, T>::get_exception() const {
    return m_exception.load();
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_otc_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  Venue OtcLinkMarketDataFeedClient<M, C, T>::get_venue(OtcLinkTier tier) {
    switch(tier) {
      case OtcLinkTier::OTCQX_US:
      case OtcLinkTier::OTCQX_INTERNATIONAL:
        return Venues::OTCQ;
      case OtcLinkTier::OTCQB:
        return Venues::OTCB;
      case OtcLinkTier::OTCID:
        return Venues::OTCD;
      case OtcLinkTier::PINK_LIMITED:
        return Venues::PINL;
      case OtcLinkTier::GREY_MARKET:
        return Venues::PSGM;
      case OtcLinkTier::EXPERT_MARKET:
        return Venues::EXPM;
      default:
        return Venue();
    }
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  boost::posix_time::ptime
      OtcLinkMarketDataFeedClient<M, C, T>::get_timestamp(
        std::uint64_t timestamp) {
    constexpr auto MAX_TIMESTAMP = std::uint64_t(253402300799999);
    if(timestamp > MAX_TIMESTAMP) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link timestamp."));
    }
    static const auto EPOCH =
      boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));
    return EPOCH + boost::posix_time::milliseconds(timestamp);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  Quantity OtcLinkMarketDataFeedClient<M, C, T>::convert(
      std::uint64_t value, std::uint64_t scale) {
    return Quantity::from_representation(
      static_cast<double>(value) * Quantity::MULTIPLIER / scale);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::log(
      const OtcLinkMessage& message, const std::exception& exception) {
    auto out = std::osyncstream(std::cout);
    out << "(bad_message " << static_cast<unsigned int>(message.m_type) <<
      ' ' << exception.what() << ' ' << std::hex << std::setfill('0');
    for(auto value : message.m_payload) {
      out << std::setw(2) <<
        static_cast<unsigned int>(static_cast<unsigned char>(value));
    }
    out << ')' << std::endl;
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  Quote OtcLinkMarketDataFeedClient<M, C, T>::get_quote(
      const Entry& entry, Side side) const {
    auto flag = [&] {
      if(side == Side::BID) {
        return OtcLinkInsideAttributes::Flag::BID_PRICED;
      }
      return OtcLinkInsideAttributes::Flag::ASK_PRICED;
    }();
    constexpr auto OPEN =
      static_cast<std::uint8_t>(OtcLinkInsideAttributes::Flag::OPEN);
    if(!m_is_market_open || (entry.m_flags & OPEN) == 0 ||
        (entry.m_flags & static_cast<std::uint8_t>(flag)) == 0) {
      return Quote(Money(), Quantity(), side);
    }
    return pick(side, entry.m_quote.m_ask, entry.m_quote.m_bid);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::publish(std::uint32_t id,
      Participant& entry, boost::posix_time::ptime timestamp) {
    auto& book = m_books.at(entry.m_security);
    for(auto side : {Side(Side::BID), Side(Side::ASK)}) {
      auto quote = get_quote(entry, side);
      auto& previous =
        pick(side, entry.m_published.m_ask, entry.m_published.m_bid);
      if(quote == previous) {
        continue;
      }
      auto order = std::to_string(id);
      if(side == Side::BID) {
        order += ":B";
      } else {
        order += ":A";
      }
      if(quote.m_size != 0) {
        m_feed_client->add_order(book.m_ticker, Venues::OTCM, entry.m_mpid,
          false, order, side, quote.m_price, quote.m_size, timestamp);
      } else if(previous.m_size != 0) {
        m_feed_client->remove_order(order, timestamp);
      }
      previous = quote;
    }
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::publish(
      Book& book, boost::posix_time::ptime timestamp) {
    auto quote = BboQuote();
    quote.m_timestamp = timestamp;
    for(auto id : book.m_insides) {
      auto& entry = m_insides.at(id);
      for(auto side : {Side(Side::BID), Side(Side::ASK)}) {
        auto candidate = get_quote(entry, side);
        auto& best = pick(side, quote.m_ask, quote.m_bid);
        if(candidate.m_size != 0 && (best.m_size == 0 ||
            listing_comparator(candidate, best))) {
          best = candidate;
        }
      }
    }
    if(quote.m_bid != book.m_published.m_bid ||
        quote.m_ask != book.m_published.m_ask) {
      m_feed_client->publish(TickerBboQuote(quote, book.m_ticker));
      book.m_published = quote;
    }
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::withdraw(std::uint32_t id,
      Participant& entry, boost::posix_time::ptime timestamp) {
    auto flags = std::exchange(entry.m_flags, std::uint8_t(0));
    publish(id, entry, timestamp);
    entry.m_flags = flags;
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::clear(
      std::uint32_t security, boost::posix_time::ptime timestamp) {
    auto i = m_books.find(security);
    if(i == m_books.end()) {
      return;
    }
    for(auto j = m_quotes.begin(); j != m_quotes.end();) {
      if(j->second.m_security == security) {
        withdraw(j->first, j->second, timestamp);
        j = m_quotes.erase(j);
      } else {
        ++j;
      }
    }
    for(auto id : i->second.m_insides) {
      m_insides.erase(id);
    }
    i->second.m_insides.clear();
    publish(i->second, timestamp);
    m_books.erase(i);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::clear(
      boost::posix_time::ptime timestamp) {
    clear_quotes(timestamp);
    m_books.clear();
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::clear_quotes(
      boost::posix_time::ptime timestamp) {
    for(auto& [id, entry] : m_quotes) {
      withdraw(id, entry, timestamp);
    }
    m_quotes.clear();
    m_insides.clear();
    for(auto& [id, book] : m_books) {
      book.m_insides.clear();
      publish(book, timestamp);
    }
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::set_market_open(
      bool is_open, boost::posix_time::ptime timestamp) {
    m_is_market_open = is_open;
    for(auto& [id, entry] : m_quotes) {
      publish(id, entry, timestamp);
    }
    for(auto& [id, book] : m_books) {
      publish(book, timestamp);
    }
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::process(
      const OtcLinkSecurity& message) {
    auto timestamp = get_timestamp(message.m_timestamp);
    auto venue = get_venue(message.m_tier);
    if(message.m_action == OtcLinkSecurityAction::DELETE ||
        message.m_status == 'D' ||
        message.m_asset_class != OtcLinkAssetClass::EQUITY ||
        message.m_symbol.empty() || !venue) {
      clear(message.m_security, timestamp);
      return;
    }
    auto ticker = Ticker(std::string(message.m_symbol), venue);
    auto [i, is_inserted] = m_books.try_emplace(message.m_security);
    auto& book = i->second;
    if(!is_inserted && book.m_ticker == ticker) {
      return;
    }
    if(!is_inserted) {
      for(auto& [id, entry] : m_quotes) {
        if(entry.m_security == message.m_security) {
          withdraw(id, entry, timestamp);
        }
      }
      if(book.m_published.m_bid.m_size != 0 ||
          book.m_published.m_ask.m_size != 0) {
        auto quote = BboQuote();
        quote.m_timestamp = timestamp;
        m_feed_client->publish(TickerBboQuote(quote, book.m_ticker));
        book.m_published = quote;
      }
    }
    book.m_ticker = std::move(ticker);
    m_feed_client->add(TickerInfo(book.m_ticker, {}, {}, {}));
    for(auto& [id, entry] : m_quotes) {
      if(entry.m_security == message.m_security) {
        publish(id, entry, timestamp);
      }
    }
    publish(book, timestamp);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  template<typename Q> requires std::same_as<Q, OtcLinkQuote> ||
    std::same_as<Q, OtcLinkFractionalQuote>
  void OtcLinkMarketDataFeedClient<M, C, T>::process(const Q& message) {
    auto timestamp =
      get_timestamp(std::max(message.m_bid_timestamp, message.m_ask_timestamp));
    auto entry = Participant();
    entry.m_security = message.m_security;
    entry.m_mpid = message.m_mpid;
    entry.m_flags = message.m_flags;
    entry.m_quote.m_bid =
      make_bid(Money(convert(message.m_bid_price, Q::PRICE_SCALE)),
        convert(message.m_bid_size, Q::SIZE_SCALE));
    entry.m_quote.m_ask =
      make_ask(Money(convert(message.m_ask_price, Q::PRICE_SCALE)),
        convert(message.m_ask_size, Q::SIZE_SCALE));
    auto i = m_quotes.find(message.m_quote);
    if(message.m_action == OtcLinkQuoteAction::DELETE) {
      if(i != m_quotes.end()) {
        withdraw(i->first, i->second, timestamp);
        m_quotes.erase(i);
      }
      return;
    }
    if(!m_books.contains(message.m_security)) {
      if(i != m_quotes.end()) {
        withdraw(i->first, i->second, timestamp);
        m_quotes.erase(i);
      }
      return;
    }
    if(i != m_quotes.end()) {
      if(i->second.m_security == entry.m_security &&
          i->second.m_mpid == entry.m_mpid) {
        entry.m_published = i->second.m_published;
      } else {
        withdraw(i->first, i->second, timestamp);
      }
    }
    auto j = m_quotes.insert_or_assign(message.m_quote, std::move(entry)).first;
    publish(j->first, j->second, timestamp);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  template<typename Q> requires std::same_as<Q, OtcLinkQuoteUpdate> ||
    std::same_as<Q, OtcLinkFractionalQuoteUpdate>
  void OtcLinkMarketDataFeedClient<M, C, T>::process(const Q& message) {
    auto i = m_quotes.find(message.m_quote);
    if(i == m_quotes.end()) {
      return;
    }
    auto timestamp = get_timestamp(message.m_timestamp);
    auto side = [&] {
      if(message.has_flag(Q::Flag::UPDATE_ASK)) {
        return Side::ASK;
      }
      return Side::BID;
    }();
    auto& entry = i->second;
    pick(side, entry.m_quote.m_ask, entry.m_quote.m_bid) =
      Quote(Money(convert(message.m_price, Q::PRICE_SCALE)),
        convert(message.m_size, Q::SIZE_SCALE), side);
    entry.m_flags = message.m_flags;
    publish(i->first, entry, timestamp);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  template<typename Q> requires std::same_as<Q, OtcLinkInside> ||
    std::same_as<Q, OtcLinkFractionalInside>
  void OtcLinkMarketDataFeedClient<M, C, T>::process(const Q& message) {
    auto timestamp = get_timestamp(
      std::max(message.m_bid_timestamp, message.m_ask_timestamp));
    auto entry = Entry();
    entry.m_security = message.m_security;
    entry.m_flags = message.m_flags;
    entry.m_quote.m_bid =
      make_bid(Money(convert(message.m_bid_price, Q::PRICE_SCALE)),
        convert(message.m_bid_size, Q::SIZE_SCALE));
    entry.m_quote.m_ask =
      make_ask(Money(convert(message.m_ask_price, Q::PRICE_SCALE)),
        convert(message.m_ask_size, Q::SIZE_SCALE));
    auto i = m_insides.find(message.m_inside);
    if(i != m_insides.end() && (
        message.m_action == OtcLinkInsideAction::DELETE ||
        i->second.m_security != message.m_security)) {
      auto& book = m_books.at(i->second.m_security);
      book.m_insides.erase(i->first);
      m_insides.erase(i);
      publish(book, timestamp);
    }
    if(message.m_action == OtcLinkInsideAction::DELETE ||
        !m_books.contains(message.m_security)) {
      return;
    }
    m_insides.insert_or_assign(message.m_inside, std::move(entry));
    auto& book = m_books.at(message.m_security);
    book.m_insides.insert(message.m_inside);
    publish(book, timestamp);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  template<typename Q> requires std::same_as<Q, OtcLinkInsideUpdate> ||
    std::same_as<Q, OtcLinkFractionalInsideUpdate>
  void OtcLinkMarketDataFeedClient<M, C, T>::process(const Q& message) {
    auto i = m_insides.find(message.m_inside);
    if(i == m_insides.end()) {
      return;
    }
    auto timestamp = get_timestamp(message.m_timestamp);
    auto side = [&] {
      if(message.has_flag(Q::Flag::UPDATE_ASK)) {
        return Side::ASK;
      }
      return Side::BID;
    }();
    auto& entry = i->second;
    pick(side, entry.m_quote.m_ask, entry.m_quote.m_bid) =
      Quote(Money(convert(message.m_price, Q::PRICE_SCALE)),
        convert(message.m_size, Q::SIZE_SCALE), side);
    entry.m_flags = message.m_flags;
    publish(m_books.at(entry.m_security), timestamp);
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::dispatch(
      const OtcLinkMessage& message) {
    visit(message, [&] (const auto& message) {
      using Message = std::remove_cvref_t<decltype(message)>;
      if constexpr(requires { process(message); }) {
        process(message);
      } else if constexpr(std::same_as<Message, OtcLinkSpinStart>) {
        if(message.m_type == OtcLinkSpinType::OPENING) {
          clear_quotes(get_timestamp(message.m_timestamp));
        }
      } else if constexpr(std::same_as<Message, OtcLinkMarketOpen>) {
        set_market_open(true, get_timestamp(message.m_timestamp));
      } else if constexpr(std::same_as<Message, OtcLinkMarketClose>) {
        set_market_open(false, get_timestamp(message.m_timestamp));
      }
    });
  }

  template<typename M, typename C, typename T> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsOtcLinkClient<Beam::dereference_t<C>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void OtcLinkMarketDataFeedClient<M, C, T>::read_loop() {
    try {
      while(m_open_state.is_open()) {
        auto session = std::uint64_t(0);
        auto message = OtcLinkMessage();
        try {
          message = m_otc_client->read(Beam::out(session));
        } catch(const Beam::EndOfFileException&) {
          break;
        }
        if(!m_open_state.is_open()) {
          break;
        }
        if(session != m_session) {
          clear(m_time_client->get_time());
          m_session = session;
          m_is_market_open = true;
        }
        try {
          dispatch(message);
        } catch(const OtcLinkParserException& exception) {
          log(message, exception);
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
