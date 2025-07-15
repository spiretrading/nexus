#ifndef NEXUS_SERVICE_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_SERVICE_MARKET_DATA_FEED_CLIENT_HPP
#include <algorithm>
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/Authenticator.hpp>
#include <Beam/ServiceLocator/ServiceEntry.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServices.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a MarketDataFeedClient using Beam services.
   * @param <O> The type used to represent order ids.
   * @param <S> The type of Timer used to sample market data sent to the
   *        servlet.
   * @param <P> The type of MessageProtocol used to send and receive messages.
   * @param <H> The type of Timer used for heartbeats.
   */
  template<typename O, typename S, typename P, typename H>
  class ServiceMarketDataFeedClient {
    public:

      /** The type used to represent order ids. */
      using OrderId = O;

      /** The type of Timer used for sampling. */
      using SamplingTimer = Beam::GetTryDereferenceType<S>;

      /** The type of MessageProtocol used to send and receive messages. */
      using MessageProtocol = P;

      /** The type of Timer used for heartbeats. */
      using HeartbeatTimer = Beam::GetTryDereferenceType<H>;

      /** The type of ServiceProtocol to use. */
      using ServiceProtocolClient = Beam::Services::ServiceProtocolClient<P, H>;

      /** The type of Authenticator to use. */
      using Authenticator = typename Beam::ServiceLocator::Authenticator<
        ServiceProtocolClient>::type;

      /**
       * Constructs a ServiceMarketDataFeedClient.
       * @param channel Initializes the Channel to the ServiceProtocol server.
       * @param authenticator The Authenticator to use.
       * @param sampling_timer Initializes the SamplingTimer.
       * @param heartbeat_timer Initializes the Timer used for heartbeats.
       */
      template<typename CF, typename SF, typename HF>
      ServiceMarketDataFeedClient(
        CF&& channel, const Authenticator& authenticator, SF&& sampling_timer,
        HF&& heartbeat_timer);
      ~ServiceMarketDataFeedClient();
      void add(const SecurityInfo& info);
      void publish(const VenueOrderImbalance& imbalance);
      void publish(const SecurityBboQuote& quote);
      void publish(const SecurityBookQuote& quote);
      void publish(const SecurityTimeAndSale& time_and_sale);
      void add_order(const Security& security, Venue venue,
        const std::string& mpid, bool is_primary_mpid, const OrderId& id,
        Side side, Money price, Quantity size,
        boost::posix_time::ptime timestamp);
      void modify_order_size(
        const OrderId& id, Quantity size, boost::posix_time::ptime timestamp);
      void offset_order_size(
        const OrderId& id, Quantity delta, boost::posix_time::ptime timestamp);
      void modify_order_price(
        const OrderId& id, Money price, boost::posix_time::ptime timestamp);
      void remove_order(const OrderId& id, boost::posix_time::ptime timestamp);
      void close();

    private:
      struct OrderEntry {
        Security m_security;
        Venue m_venue;
        std::string m_mpid;
        bool m_is_primary_mpid;
        Side m_side;
        Money m_price;
        Quantity m_size;
      };
      struct QuoteUpdates {
        boost::optional<SecurityBboQuote> m_bbo_quote;
        std::vector<SecurityBookQuote> m_asks;
        std::vector<SecurityBookQuote> m_bids;
        std::vector<SecurityTimeAndSale> m_time_and_sales;
      };
      mutable boost::mutex m_mutex;
      ServiceProtocolClient m_client;
      Beam::GetOptionalLocalPtr<S> m_sampling_timer;
      std::unordered_map<Security, QuoteUpdates> m_quote_updates;
      std::vector<VenueOrderImbalance> m_order_imbalances;
      std::unordered_map<OrderId, OrderEntry> m_orders;
      Beam::IO::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      ServiceMarketDataFeedClient(const ServiceMarketDataFeedClient&) = delete;
      ServiceMarketDataFeedClient& operator =(
        const ServiceMarketDataFeedClient&) = delete;
      void update_book_sampling(const SecurityBookQuote& quote);
      void locked_add_order(const Security& security, Venue venue,
        const std::string& mpid, bool is_primary_mpid, const OrderId& id,
        Side side, Money price, Quantity size,
        boost::posix_time::ptime timestamp);
      void locked_remove_order(
        typename std::unordered_map<OrderId, OrderEntry>::iterator& i,
        boost::posix_time::ptime timestamp);
      void on_timer_expired(Beam::Threading::Timer::Result result);
  };

  /**
   * Finds a MarketDataFeedService for a specified country.
   * @param country The country to service.
   * @param client The ServiceLocatorClient used to locate services.
   * @return The ServiceEntry belonging to the MarketDataFeedService for the
   *         specified <i>country</i>.
   */
  template<typename ServiceLocatorClient>
  boost::optional<Beam::ServiceLocator::ServiceEntry>
      find_market_data_feed_service(
        CountryCode country, ServiceLocatorClient& client) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto services = client.Locate(FEED_SERVICE_NAME);
      for(auto& entry : services) {
        auto& properties = entry.GetProperties();
        auto countries_property = properties.Get("countries");
        if(!countries_property) {
          return boost::make_optional(entry);
        } else if(auto countries =
            boost::get<std::vector<Beam::JsonValue>>(&*countries_property)) {
          for(auto country : *countries) {
            if(auto value = boost::get<double>(&country)) {
              if(static_cast<CountryCode>(
                  static_cast<std::uint16_t>(*value)) == country) {
                return boost::make_optional(entry);
              }
            }
          }
        }
      }
      return boost::optional<Beam::ServiceLocator::ServiceEntry>();
    }, "Failed to find market data feed service: " +
      boost::lexical_cast<std::string>(country));
  }

  template<typename O, typename S, typename P, typename H>
  template<typename CF, typename SF, typename HF>
  ServiceMarketDataFeedClient<O, S, P, H>::ServiceMarketDataFeedClient(
      CF&& channel, const Authenticator& authenticator, SF&& sampling_timer,
      HF&& heartbeat_timer)
      try : m_client(std::forward<CF>(channel),
              std::forward<HF>(heartbeat_timer)),
            m_sampling_timer(std::forward<SF>(sampling_timer)) {
    RegisterMarketDataFeedMessages(Beam::Store(m_client.GetSlots()));
    try {
      Beam::ServiceLocator::Authenticate(authenticator, m_client);
      m_sampling_timer->GetPublisher().Monitor(
        m_tasks.GetSlot<Beam::Threading::Timer::Result>(std::bind_front(
          &ServiceMarketDataFeedClient::on_timer_expired, this)));
      m_sampling_timer->Start();
    } catch(const std::exception&) {
      close();
      BOOST_RETHROW;
    }
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the market data feed server."));
  }

  template<typename O, typename S, typename P, typename H>
  ServiceMarketDataFeedClient<O, S, P, H>::~ServiceMarketDataFeedClient() {
    close();
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::add(const SecurityInfo& info) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      Beam::Services::SendRecordMessage<SetSecurityInfoMessage>(m_client, info);
    }, "Failed to add security info: " +
      boost::lexical_cast<std::string>(info));
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::publish(
      const VenueOrderImbalance& imbalance) {
    auto lock = boost::lock_guard(m_mutex);
    m_order_imbalances.push_back(imbalance);
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::publish(
      const SecurityBboQuote& quote) {
    auto lock = boost::lock_guard(m_mutex);
    auto& updates = m_quote_updates[quote.GetIndex()];
    updates.m_bbo_quote = quote;
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::publish(
      const SecurityBookQuote& quote) {
    auto id = quote.GetIndex().get_symbol() + '-' +
      boost::lexical_cast<std::string>(quote.GetIndex().get_venue()) +
      quote->m_mpid + '-' +
      boost::lexical_cast<std::string>(quote->m_quote.m_price) +
      to_char(quote->m_quote.m_side);
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      locked_add_order(quote.GetIndex(), quote->m_venue, quote->m_mpid,
        quote->m_is_primary_mpid, id, quote->m_quote.m_side,
        quote->m_quote.m_price, quote->m_quote.m_size, quote->m_timestamp);
    } else {
      locked_remove_order(i, quote->m_timestamp);
      if(quote->m_quote.m_size != 0) {
        locked_add_order(quote.GetIndex(), quote->m_venue, quote->m_mpid,
          quote->m_is_primary_mpid, id, quote->m_quote.m_side,
          quote->m_quote.m_price, quote->m_quote.m_size, quote->m_timestamp);
      }
    }
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::publish(
      const SecurityTimeAndSale& time_and_sale) {
    auto lock = boost::lock_guard(m_mutex);
    auto& updates = m_quote_updates[time_and_sale.GetIndex()];
    updates.m_time_and_sales.push_back(time_and_sale);
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::add_order(
      const Security& security, Venue venue, const std::string& mpid,
      bool is_primary_mpid, const OrderId& id, Side side, Money price,
      Quantity size, boost::posix_time::ptime timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    locked_add_order(security, venue, mpid, is_primary_mpid, id, side, price,
      size, timestamp);
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::modify_order_size(
      const OrderId& id, Quantity size, boost::posix_time::ptime timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return;
    }
    auto entry = i->second;
    locked_remove_order(i, timestamp);
    locked_add_order(entry.m_security, entry.m_venue, entry.m_mpid,
      entry.m_is_primary_mpid, id, entry.m_side, entry.m_price, size,
      timestamp);
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::offset_order_size(
      const OrderId& id, Quantity delta, boost::posix_time::ptime timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return;
    }
    auto entry = i->second;
    locked_remove_order(i, timestamp);
    locked_add_order(entry.m_security, entry.m_venue, entry.m_mpid,
      entry.m_is_primary_mpid, id, entry.m_side, entry.m_price,
      entry.m_size + delta, timestamp);
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::modify_order_price(
      const OrderId& id, Money price, boost::posix_time::ptime timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return;
    }
    auto entry = i->second;
    locked_remove_order(i, timestamp);
    locked_add_order(entry.m_security, entry.m_venue, entry.m_mpid,
      entry.m_is_primary_mpid, id, entry.m_side, price, entry.m_size,
      timestamp);
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::remove_order(
      const OrderId& id, boost::posix_time::ptime timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return;
    }
    locked_remove_order(i, timestamp);
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_sampling_timer->Cancel();
    m_tasks.Break();
    m_tasks.Wait();
    m_client.Close();
    m_open_state.Close();
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::update_book_sampling(
      const SecurityBookQuote& quote) {
    auto& book = pick(quote->m_quote.m_side,
      m_quote_updates[quote.GetIndex()].m_asks,
      m_quote_updates[quote.GetIndex()].m_bids);
    auto i = std::lower_bound(book.begin(), book.end(), quote,
      [] (const auto& lhs, const auto& rhs) {
        return listing_comparator(lhs, rhs);
      });
    if(i == book.end()) {
      book.push_back(quote);
    } else if((*i)->m_quote.m_price == quote->m_quote.m_price &&
        (*i)->m_mpid == quote->m_mpid) {
      (*i)->m_quote.m_size += quote->m_quote.m_size;
      (*i)->m_timestamp = quote->m_timestamp;
    } else {
      book.insert(i, quote);
    }
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::locked_add_order(
      const Security& security, Venue venue, const std::string& mpid,
      bool is_primary_mpid, const OrderId& id, Side side, Money price,
      Quantity size, boost::posix_time::ptime timestamp) {
    if(size <= 0) {
      return;
    }
    auto i = m_orders.find(id);
    if(i != m_orders.end()) {
      locked_remove_order(i, timestamp);
    }
    m_orders.insert(std::pair(id,
      OrderEntry(security, venue, mpid, is_primary_mpid, side, price, size)));
    auto quote = BookQuote(
      mpid, is_primary_mpid, venue, Quote(price, size, side), timestamp);
    update_book_sampling(
      Beam::Queries::IndexedValue(std::move(quote), security));
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::locked_remove_order(
      typename std::unordered_map<OrderId, OrderEntry>::iterator& i,
      boost::posix_time::ptime timestamp) {
    auto& entry = i->second;
    auto quote = BookQuote(entry.m_mpid, entry.m_is_primary_mpid, entry.m_venue,
      Quote(entry.m_price, -entry.m_size, entry.m_side), timestamp);
    update_book_sampling(
      Beam::Queries::IndexedValue(std::move(quote), entry.m_security));
    m_orders.erase(i);
  }

  template<typename O, typename S, typename P, typename H>
  void ServiceMarketDataFeedClient<O, S, P, H>::on_timer_expired(
      Beam::Threading::Timer::Result result) {
    auto messages = std::vector<MarketDataFeedMessage>();
    auto quote_updates = std::unordered_map<Security, QuoteUpdates>();
    auto order_imbalances = std::vector<VenueOrderImbalance>();
    {
      auto lock = boost::lock_guard(m_mutex);
      quote_updates.swap(m_quote_updates);
      order_imbalances.swap(m_order_imbalances);
    }
    for(auto& [security, updates] : quote_updates) {
      if(updates.m_bbo_quote) {
        messages.push_back(std::move(*updates.m_bbo_quote));
      }
      std::copy_if(std::make_move_iterator(updates.m_asks.begin()),
        std::make_move_iterator(updates.m_asks.end()),
        std::back_inserter(messages), [] (const auto& quote) {
          return quote->m_quote.m_size != 0;
        });
      std::copy_if(std::make_move_iterator(updates.m_bids.begin()),
        std::make_move_iterator(updates.m_bids.end()),
        std::back_inserter(messages), [] (const auto& quote) {
          return quote->m_quote.m_size != 0;
        });
      std::move(updates.m_time_and_sales.begin(),
        updates.m_time_and_sales.end(), std::back_inserter(messages));
    }
    std::move(order_imbalances.begin(), order_imbalances.end(),
      std::back_inserter(messages));
    if(!messages.empty()) {
      Beam::Services::SendRecordMessage<SendMarketDataFeedMessages>(
        m_client, messages);
    }
    m_sampling_timer->Start();
  }
}

#endif
