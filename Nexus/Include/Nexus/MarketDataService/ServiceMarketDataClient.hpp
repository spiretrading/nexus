#ifndef NEXUS_SERVICE_MARKET_DATA_CLIENT_HPP
#define NEXUS_SERVICE_MARKET_DATA_CLIENT_HPP
#include <exception>
#include <functional>
#include <utility>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/QueryClientPublisher.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Services/ServiceRequestException.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {

  /**
   * Implements a MarketDataClient using Beam services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceMarketDataClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::dereference_t<B>;

      /**
       * Constructs a ServiceMarketDataClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<Beam::Initializes<B> BF>
      explicit ServiceMarketDataClient(BF&& client_builder);

      ~ServiceMarketDataClient();

      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);
      std::vector<SecurityInfo> query(const SecurityInfoQuery& query);
      SecuritySnapshot load_snapshot(const Security& security);
      SecurityTechnicals load_technicals(const Security& security);
      std::vector<SecurityInfo> load_security_info_from_prefix(
        const std::string& prefix);
      void close();

    private:
      template<typename Value, typename Query, typename QueryService,
        typename EndQueryMessage>
      using QueryClientPublisher =
        Beam::QueryClientPublisher<Value, Query, EvaluatorTranslator,
          Beam::ServiceProtocolClientHandler<B>, QueryService, EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::ServiceProtocolClientHandler<B> m_client_handler;
      QueryClientPublisher<OrderImbalance, VenueMarketDataQuery,
        QueryOrderImbalancesService, EndOrderImbalanceQueryMessage>
          m_order_imbalance_publisher;
      QueryClientPublisher<BboQuote, SecurityMarketDataQuery,
        QueryBboQuotesService, EndBboQuoteQueryMessage> m_bbo_quote_publisher;
      QueryClientPublisher<BookQuote, SecurityMarketDataQuery,
        QueryBookQuotesService, EndBookQuoteQueryMessage>
          m_book_quote_publisher;
      QueryClientPublisher<TimeAndSale, SecurityMarketDataQuery,
        QueryTimeAndSalesService, EndTimeAndSaleQueryMessage>
          m_time_and_sale_publisher;
      Beam::OpenState m_open_state;

      ServiceMarketDataClient(const ServiceMarketDataClient&) = delete;
      ServiceMarketDataClient& operator =(
        const ServiceMarketDataClient&) = delete;
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
  };

  template<typename B>
  template<Beam::Initializes<B> BF>
  ServiceMarketDataClient<B>::ServiceMarketDataClient(BF&& client_builder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_client_handler(std::forward<BF>(client_builder),
              std::bind_front(&ServiceMarketDataClient::on_reconnect, this)),
            m_order_imbalance_publisher(Beam::Ref(m_client_handler)),
            m_bbo_quote_publisher(Beam::Ref(m_client_handler)),
            m_book_quote_publisher(Beam::Ref(m_client_handler)),
            m_time_and_sale_publisher(Beam::Ref(m_client_handler)) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    Nexus::register_query_types(
      Beam::out(m_client_handler.get_slots().get_registry()));
    register_market_data_registry_services(
      Beam::out(m_client_handler.get_slots()));
    register_market_data_registry_messages(
      Beam::out(m_client_handler.get_slots()));
    m_order_imbalance_publisher.
      template add_message_handler<OrderImbalanceMessage>();
    m_bbo_quote_publisher.template add_message_handler<BboQuoteMessage>();
    m_book_quote_publisher.template add_message_handler<BookQuoteMessage>();
    m_time_and_sale_publisher.
      template add_message_handler<TimeAndSaleMessage>();
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to connect to the market data server."));
  }

  template<typename B>
  ServiceMarketDataClient<B>::~ServiceMarketDataClient() {
    close();
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_order_imbalance_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_order_imbalance_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_bbo_quote_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_bbo_quote_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_book_quote_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_book_quote_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_time_and_sale_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_time_and_sale_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  std::vector<SecurityInfo> ServiceMarketDataClient<B>::query(
      const SecurityInfoQuery& query) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<QuerySecurityInfoService>(query);
    }, "Failed to query for security info records: " +
      boost::lexical_cast<std::string>(query));
  }

  template<typename B>
  SecuritySnapshot ServiceMarketDataClient<B>::load_snapshot(
      const Security& security) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadSecuritySnapshotService>(
        security);
    }, "Failed to load security snapshot: " +
      boost::lexical_cast<std::string>(security));
  }

  template<typename B>
  SecurityTechnicals ServiceMarketDataClient<B>::load_technicals(
      const Security& security) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadSecurityTechnicalsService>(
        security);
    }, "Failed to load security technicals: " +
      boost::lexical_cast<std::string>(security));
  }

  template<typename B>
  std::vector<SecurityInfo>
      ServiceMarketDataClient<B>::load_security_info_from_prefix(
        const std::string& prefix) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadSecurityInfoFromPrefixService>(
        prefix);
    }, "Failed to load security info from prefix: \"" + prefix + "\"");
  }

  template<typename B>
  void ServiceMarketDataClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_client_handler.close();
    m_order_imbalance_publisher.close();
    m_bbo_quote_publisher.close();
    m_book_quote_publisher.close();
    m_time_and_sale_publisher.close();
    m_open_state.close();
  }

  template<typename B>
  void ServiceMarketDataClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_order_imbalance_publisher.recover(*client);
    m_bbo_quote_publisher.recover(*client);
    m_book_quote_publisher.recover(*client);
    m_time_and_sale_publisher.recover(*client);
  }
}

#endif
