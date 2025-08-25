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
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a ServiceMarketDataClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
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
      using QueryClientPublisher = Beam::Queries::QueryClientPublisher<
        Value, Query, EvaluatorTranslator,
        Beam::Services::ServiceProtocolClientHandler<B>, QueryService,
        EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_client_handler;
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
      Beam::IO::OpenState m_open_state;

      ServiceMarketDataClient(const MarketDataClient&) = delete;
      ServiceMarketDataClient& operator =(const MarketDataClient&) = delete;
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
  };

  template<typename B>
  template<typename BF>
  ServiceMarketDataClient<B>::ServiceMarketDataClient(BF&& client_builder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_client_handler(std::forward<BF>(client_builder),
              std::bind_front(&ServiceMarketDataClient::on_reconnect, this)),
            m_order_imbalance_publisher(Beam::Ref(m_client_handler)),
            m_bbo_quote_publisher(Beam::Ref(m_client_handler)),
            m_book_quote_publisher(Beam::Ref(m_client_handler)),
            m_time_and_sale_publisher(Beam::Ref(m_client_handler)) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    RegisterQueryTypes(Beam::Store(m_client_handler.GetSlots().GetRegistry()));
    RegisterMarketDataRegistryServices(
      Beam::Store(m_client_handler.GetSlots()));
    RegisterMarketDataRegistryMessages(
      Beam::Store(m_client_handler.GetSlots()));
    m_order_imbalance_publisher.
      template AddMessageHandler<OrderImbalanceMessage>();
    m_bbo_quote_publisher.template AddMessageHandler<BboQuoteMessage>();
    m_book_quote_publisher.template AddMessageHandler<BookQuoteMessage>();
    m_time_and_sale_publisher.template AddMessageHandler<TimeAndSaleMessage>();
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the market data server."));
  }

  template<typename B>
  ServiceMarketDataClient<B>::~ServiceMarketDataClient() {
    close();
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_order_imbalance_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_order_imbalance_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_bbo_quote_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_bbo_quote_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_book_quote_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_book_quote_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_time_and_sale_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceMarketDataClient<B>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_time_and_sale_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  std::vector<SecurityInfo> ServiceMarketDataClient<B>::query(
      const SecurityInfoQuery& query) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<QuerySecurityInfoService>(query);
    }, "Failed to query for security info records: " +
      boost::lexical_cast<std::string>(query));
  }

  template<typename B>
  SecuritySnapshot ServiceMarketDataClient<B>::load_snapshot(
      const Security& security) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadSecuritySnapshotService>(
        security);
    }, "Failed to load security snapshot: " +
      boost::lexical_cast<std::string>(security));
  }

  template<typename B>
  SecurityTechnicals ServiceMarketDataClient<B>::load_technicals(
      const Security& security) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadSecurityTechnicalsService>(
        security);
    }, "Failed to load security technicals: " +
      boost::lexical_cast<std::string>(security));
  }

  template<typename B>
  std::vector<SecurityInfo>
      ServiceMarketDataClient<B>::load_security_info_from_prefix(
        const std::string& prefix) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadSecurityInfoFromPrefixService>(
        prefix);
    }, "Failed to load security info from prefix: \"" + prefix + "\"");
  }

  template<typename B>
  void ServiceMarketDataClient<B>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_client_handler.Close();
    m_order_imbalance_publisher.Break();
    m_bbo_quote_publisher.Break();
    m_book_quote_publisher.Break();
    m_time_and_sale_publisher.Break();
    m_open_state.Close();
  }

  template<typename B>
  void ServiceMarketDataClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_order_imbalance_publisher.Recover(*client);
    m_bbo_quote_publisher.Recover(*client);
    m_book_quote_publisher.Recover(*client);
    m_time_and_sale_publisher.Recover(*client);
  }
}

#endif
