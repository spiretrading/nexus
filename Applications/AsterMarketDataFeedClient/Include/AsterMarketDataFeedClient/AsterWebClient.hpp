#ifndef ASTER_WEB_CLIENT_HPP
#define ASTER_WEB_CLIENT_HPP
#include <atomic>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/WebServices/HttpClient.hpp>
#include <Beam/WebServices/WebSocket.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/MarketDataService/BboQuoteModel.hpp"

namespace Nexus {

  /**
   * Client used to access the Aster market data REST API.
   * @tparam C The type of Channel used to connect to the server.
   */
  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  class AsterWebClient {
    public:

      /**
       * The factory used to build Channels.
       * @param uri The URI to connect to.
       * @return A Channel able to connect to the specified uri.
       */
      using ChannelBuilder = std::function<C (const Beam::Uri& uri)>;

      /**
       * Constructs an AsterWebClient.
       * @param builder The ChannelBuilder used to connect to the server.
       */
      explicit AsterWebClient(const ChannelBuilder& builder);

      /**
       * Constructs an AsterWebClient.
       * @param builder The ChannelBuilder used to connect to the server.
       * @param http_uri The endpoint used for HTTP services.
       * @param web_socket_uri The endpoint used for WebSocket services.
       */
      explicit AsterWebClient(const ChannelBuilder& builder,
        Beam::Uri http_uri, Beam::Uri web_socket_uri);

      ~AsterWebClient();

      /** Tests connectivity to the REST API. */
      void ping();

      /** Returns the server's current time. */
      boost::posix_time::ptime load_server_time();

      /**
       * Subscribes to bbo quote updates for a ticker.
       * @param ticker The ticker to subscribe to.
       * @param queue The queue to publish BboQuotes to.
       */
      void subscribe(
        const Ticker& ticker, Beam::ScopedQueueWriter<BboQuote> queue);

      /**
       * Subscribes to book quote updates for a ticker.
       * @param ticker The ticker to subscribe to.
       * @param queue The queue to publish BookQuotes to.
       */
      void subscribe(
        const Ticker& ticker, Beam::ScopedQueueWriter<BookQuote> queue);

      /**
       * Subscribes to time and sale updates for a ticker.
       * @param ticker The ticker to subscribe to.
       * @param queue The queue to publish TimeAndSales to.
       */
      void subscribe(
        const Ticker& ticker, Beam::ScopedQueueWriter<TimeAndSale> queue);

      void close();

    private:
      struct BboSubscriptionEntry {
        std::vector<Beam::ScopedQueueWriter<BboQuote>> m_queues;
        BboQuoteModel m_model;
      };
      Beam::HttpClient<C> m_http_client;
      Beam::WebSocket<C> m_web_socket;
      std::atomic_int m_next_subscription_id;
      Beam::Mutex m_mutex;
      std::unordered_map<std::string, BboSubscriptionEntry>
        m_bbo_subscriptions;
      std::unordered_map<std::string,
        std::vector<Beam::ScopedQueueWriter<BookQuote>>>
          m_book_quote_subscriptions;
      std::unordered_map<std::string,
        std::vector<Beam::ScopedQueueWriter<TimeAndSale>>>
          m_time_and_sale_subscriptions;
      Beam::RoutineHandler m_web_socket_read_handler;
      Beam::OpenState m_open_state;
      Beam::Uri m_http_uri;

      static boost::posix_time::ptime to_ptime(std::uint64_t milliseconds);
      AsterWebClient(const AsterWebClient&) = delete;
      AsterWebClient& operator =(const AsterWebClient&) = delete;
      Beam::Uri make_http_uri(std::string_view path) const;
      void send_subscribe(const std::string& stream);
      void send_unsubscribe(const std::string& stream);
      void on_depth_update(const Beam::SharedBuffer& message);
      void on_agg_trade(const Beam::SharedBuffer& message);
      void web_socket_read_loop();
  };

  template<typename F>
  AsterWebClient(F, Beam::Uri, Beam::Uri) ->
    AsterWebClient<std::invoke_result_t<F, const Beam::Uri&>>;

  template<typename F>
  AsterWebClient(F) ->
    AsterWebClient<std::invoke_result_t<F, const Beam::Uri&>>;

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  AsterWebClient<C>::AsterWebClient(const ChannelBuilder& builder)
    : AsterWebClient(builder, Beam::Uri("https://fapi.binance.com"),
        Beam::Uri("wss://fstream.asterdex.com/ws")) {}

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  AsterWebClient<C>::AsterWebClient(const ChannelBuilder& builder,
      Beam::Uri http_uri, Beam::Uri web_socket_uri)
      : m_http_client(builder),
        m_web_socket(Beam::WebSocketConfig().set_uri(web_socket_uri), builder),
        m_next_subscription_id(0),
        m_http_uri(http_uri) {
    m_web_socket_read_handler = Beam::spawn(
      std::bind_front(&AsterWebClient::web_socket_read_loop, this));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  AsterWebClient<C>::~AsterWebClient() {
    close();
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::ping() {
    auto request = Beam::HttpRequest(Beam::HttpMethod::GET,
      make_http_uri("/fapi/v1/ping"));
    auto response = m_http_client.send(request);
    if(response.get_status_code() != Beam::HttpStatusCode::OK) {
      throw std::runtime_error("Ping failed.");
    }
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  boost::posix_time::ptime AsterWebClient<C>::load_server_time() {
    struct Result {
      std::uint64_t m_server_time;

      void shuttle(Beam::JsonReceiver<Beam::SharedBuffer>& receiver,
          unsigned int version) {
        receiver.shuttle("serverTime", m_server_time);
      }
    };
    auto request = Beam::HttpRequest(Beam::HttpMethod::GET,
      make_http_uri("/fapi/v1/time"));
    auto response = m_http_client.send(request);
    if(response.get_status_code() != Beam::HttpStatusCode::OK) {
      throw std::runtime_error("Load server time failed.");
    }
    auto receiver = Beam::JsonReceiver<Beam::SharedBuffer>();
    receiver.set(Beam::Ref(response.get_body()));
    auto result = Beam::receive<Result>(receiver);
    return to_ptime(result.m_server_time);
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::subscribe(
      const Ticker& ticker, Beam::ScopedQueueWriter<BboQuote> queue) {
    auto stream = boost::to_lower_copy(ticker.get_symbol()) + "@depth";
    auto lock = std::lock_guard(m_mutex);
    auto& entry = m_bbo_subscriptions[stream];
    if(entry.m_queues.empty() &&
        m_book_quote_subscriptions.find(stream) ==
          m_book_quote_subscriptions.end()) {
      send_subscribe(stream);
    }
    entry.m_queues.push_back(std::move(queue));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::subscribe(
      const Ticker& ticker, Beam::ScopedQueueWriter<BookQuote> queue) {
    auto stream = boost::to_lower_copy(ticker.get_symbol()) + "@depth";
    auto lock = std::lock_guard(m_mutex);
    auto& queues = m_book_quote_subscriptions[stream];
    if(queues.empty() && m_bbo_subscriptions.find(stream) ==
        m_bbo_subscriptions.end()) {
      send_subscribe(stream);
    }
    queues.push_back(std::move(queue));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::subscribe(
      const Ticker& ticker, Beam::ScopedQueueWriter<TimeAndSale> queue) {
    auto stream = boost::to_lower_copy(ticker.get_symbol()) + "@aggTrade";
    auto lock = std::lock_guard(m_mutex);
    auto& queues = m_time_and_sale_subscriptions[stream];
    if(queues.empty()) {
      send_subscribe(stream);
    }
    queues.push_back(std::move(queue));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_web_socket.close();
    m_web_socket_read_handler.wait();
    {
      auto lock = std::lock_guard(m_mutex);
      m_bbo_subscriptions.clear();
      m_book_quote_subscriptions.clear();
      m_time_and_sale_subscriptions.clear();
    }
    m_open_state.close();
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  boost::posix_time::ptime AsterWebClient<C>::to_ptime(
      std::uint64_t milliseconds) {
    return boost::posix_time::from_time_t(milliseconds / 1000) +
      boost::posix_time::milliseconds(milliseconds % 1000);
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::send_subscribe(const std::string& stream) {
    struct Parameters {
      std::vector<std::string> m_params;
      int m_id;

      void shuttle(
          Beam::JsonSender<Beam::SharedBuffer>& shuttle, unsigned int version) {
        shuttle.shuttle("method", std::string("SUBSCRIBE"));
        shuttle.shuttle("params", m_params);
        shuttle.shuttle("id", m_id);
      }
    };
    auto sender = Beam::JsonSender<Beam::SharedBuffer>();
    auto body = Beam::SharedBuffer();
    sender.set(Beam::Ref(body));
    auto parameters = Parameters();
    parameters.m_params.push_back(stream);
    parameters.m_id = ++m_next_subscription_id;
    sender.send_unversioned(parameters);
    m_web_socket.write(body);
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::send_unsubscribe(const std::string& stream) {
    struct Parameters {
      std::vector<std::string> m_params;
      int m_id;

      void shuttle(
          Beam::JsonSender<Beam::SharedBuffer>& shuttle, unsigned int version) {
        shuttle.shuttle("method", std::string("UNSUBSCRIBE"));
        shuttle.shuttle("params", m_params);
        shuttle.shuttle("id", m_id);
      }
    };
    auto sender = Beam::JsonSender<Beam::SharedBuffer>();
    auto body = Beam::SharedBuffer();
    sender.set(Beam::Ref(body));
    auto parameters = Parameters();
    parameters.m_params.push_back(stream);
    parameters.m_id = ++m_next_subscription_id;
    sender.send_unversioned(parameters);
    m_web_socket.write(body);
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::on_depth_update(const Beam::SharedBuffer& message) {
    struct DepthUpdate {
      std::string m_event;
      std::uint64_t m_event_time;
      std::uint64_t m_transaction_time;
      std::string m_symbol;
      std::uint64_t m_first_update_id;
      std::uint64_t m_final_update_id;
      std::uint64_t m_previous_final_update_id;
      std::vector<std::array<std::string, 2>> m_bids;
      std::vector<std::array<std::string, 2>> m_asks;

      void shuttle(Beam::JsonReceiver<Beam::SharedBuffer>& receiver,
          unsigned int version) {
        receiver.shuttle("e", m_event);
        receiver.shuttle("E", m_event_time);
        receiver.shuttle("T", m_transaction_time);
        receiver.shuttle("s", m_symbol);
        receiver.shuttle("U", m_first_update_id);
        receiver.shuttle("u", m_final_update_id);
        receiver.shuttle("pu", m_previous_final_update_id);
        receiver.shuttle("b", m_bids);
        receiver.shuttle("a", m_asks);
      }
    };
    auto receiver = Beam::JsonReceiver<Beam::SharedBuffer>();
    receiver.set(Beam::Ref(message));
    auto update = Beam::receive<DepthUpdate>(receiver);
    auto timestamp = to_ptime(update.m_event_time);
    auto quotes = std::vector<BookQuote>();
    for(auto& bid : update.m_bids) {
      auto quote = BookQuote();
      quote.m_mpid = "ASTR";
      quote.m_is_primary_mpid = true;
      quote.m_venue = Venue("ASTR");
      quote.m_quote.m_side = Side::BID;
      quote.m_quote.m_price = parse_money(bid[0]);
      quote.m_quote.m_size = parse_quantity(bid[1]);
      quote.m_timestamp = timestamp;
      quotes.push_back(std::move(quote));
    }
    for(auto& ask : update.m_asks) {
      auto quote = BookQuote();
      quote.m_mpid = "ASTR";
      quote.m_is_primary_mpid = true;
      quote.m_venue = Venue("ASTR");
      quote.m_quote.m_side = Side::ASK;
      quote.m_quote.m_price = parse_money(ask[0]);
      quote.m_quote.m_size = parse_quantity(ask[1]);
      quote.m_timestamp = timestamp;
      quotes.push_back(std::move(quote));
    }
    auto stream = boost::to_lower_copy(update.m_symbol) + "@depth";
    auto lock = std::lock_guard(m_mutex);
    auto book_subscription = m_book_quote_subscriptions.find(stream);
    if(book_subscription != m_book_quote_subscriptions.end()) {
      auto& queues = book_subscription->second;
      auto iterator = queues.begin();
      while(iterator != queues.end()) {
        try {
          for(auto& quote : quotes) {
            iterator->push(quote);
          }
          ++iterator;
        } catch(const std::exception&) {
          iterator = queues.erase(iterator);
        }
      }
      if(queues.empty()) {
        m_book_quote_subscriptions.erase(book_subscription);
      }
    }
    auto bbo_subscription = m_bbo_subscriptions.find(stream);
    if(bbo_subscription != m_bbo_subscriptions.end()) {
      auto& model = bbo_subscription->second.m_model;
      auto changed = false;
      for(auto& quote : quotes) {
        if(model.update(quote)) {
          changed = true;
        }
      }
      if(changed) {
        auto& queues = bbo_subscription->second.m_queues;
        auto iterator = queues.begin();
        while(iterator != queues.end()) {
          try {
            iterator->push(model.get_bbo());
            ++iterator;
          } catch(const std::exception&) {
            iterator = queues.erase(iterator);
          }
        }
        if(queues.empty()) {
          m_bbo_subscriptions.erase(bbo_subscription);
        }
      }
    }
    if(m_book_quote_subscriptions.find(stream) ==
        m_book_quote_subscriptions.end() &&
        m_bbo_subscriptions.find(stream) ==
          m_bbo_subscriptions.end()) {
      send_unsubscribe(stream);
    }
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::on_agg_trade(const Beam::SharedBuffer& message) {
    struct AggTrade {
      std::string m_event;
      std::uint64_t m_event_time;
      std::string m_symbol;
      std::uint64_t m_aggregate_trade_id;
      std::string m_price;
      std::string m_quantity;
      std::uint64_t m_first_trade_id;
      std::uint64_t m_last_trade_id;
      std::uint64_t m_trade_time;
      bool m_is_buyer_maker;

      void shuttle(Beam::JsonReceiver<Beam::SharedBuffer>& receiver,
          unsigned int version) {
        receiver.shuttle("e", m_event);
        receiver.shuttle("E", m_event_time);
        receiver.shuttle("s", m_symbol);
        receiver.shuttle("a", m_aggregate_trade_id);
        receiver.shuttle("p", m_price);
        receiver.shuttle("q", m_quantity);
        receiver.shuttle("f", m_first_trade_id);
        receiver.shuttle("l", m_last_trade_id);
        receiver.shuttle("T", m_trade_time);
        receiver.shuttle("m", m_is_buyer_maker);
      }
    };
    auto receiver = Beam::JsonReceiver<Beam::SharedBuffer>();
    receiver.set(Beam::Ref(message));
    auto trade = Beam::receive<AggTrade>(receiver);
    auto time_and_sale = TimeAndSale();
    time_and_sale.m_timestamp = to_ptime(trade.m_trade_time);
    time_and_sale.m_price = parse_money(trade.m_price);
    time_and_sale.m_size = parse_quantity(trade.m_quantity);
    time_and_sale.m_condition.m_type = TimeAndSale::Condition::Type::REGULAR;
    time_and_sale.m_market_center = "ASTR";
    auto stream = boost::to_lower_copy(trade.m_symbol) + "@aggTrade";
    auto lock = std::lock_guard(m_mutex);
    auto subscription = m_time_and_sale_subscriptions.find(stream);
    if(subscription == m_time_and_sale_subscriptions.end()) {
      return;
    }
    auto& queues = subscription->second;
    auto iterator = queues.begin();
    while(iterator != queues.end()) {
      try {
        iterator->push(time_and_sale);
        ++iterator;
      } catch(const std::exception&) {
        iterator = queues.erase(iterator);
      }
    }
    if(queues.empty()) {
      m_time_and_sale_subscriptions.erase(subscription);
      send_unsubscribe(stream);
    }
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  Beam::Uri AsterWebClient<C>::make_http_uri(std::string_view path) const {
    auto uri = std::string(m_http_uri.get_scheme());
    uri += "://";
    uri += m_http_uri.get_hostname();
    auto port = m_http_uri.get_port();
    if(port != 0) {
      uri += ":";
      uri += std::to_string(port);
    }
    uri += std::string(path);
    return Beam::Uri(uri);
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void AsterWebClient<C>::web_socket_read_loop() {
    static const auto DEPTH_UPDATE_PREFIX =
      std::string_view("{\"e\":\"depthUpdate\"");
    static const auto AGG_TRADE_PREFIX =
      std::string_view("{\"e\":\"aggTrade\"");
    while(true) {
      try {
        auto message = m_web_socket.read();
        auto view =
          std::string_view(message.get_data(), message.get_size());
        if(view.starts_with(DEPTH_UPDATE_PREFIX)) {
          on_depth_update(message);
        } else if(view.starts_with(AGG_TRADE_PREFIX)) {
          on_agg_trade(message);
        } else {
          std::cout << view << std::endl;
        }
      } catch(const Beam::EndOfFileException&) {
        break;
      } catch(const std::exception& e) {
        std::cerr << "AsterWebClient WebSocket read error: " << e.what() <<
          std::endl;
      }
    }
  }
}

#endif
