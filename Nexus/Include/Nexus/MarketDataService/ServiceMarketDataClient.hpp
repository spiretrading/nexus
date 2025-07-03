#ifndef NEXUS_SERVICE_MARKET_DATA_CLIENT_HPP
#define NEXUS_SERVICE_MARKET_DATA_CLIENT_HPP
#include <functional>
#include <utility>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/QueryClientPublisher.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus::MarketDataService {

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
        Value, Query, Queries::EvaluatorTranslator,
        Beam::Services::ServiceProtocolClientHandler<B>, QueryService,
        EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
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
      Beam::IO::OpenState m_openState;

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
    Queries::RegisterQueryTypes(
      Beam::Store(m_client_handler.GetSlots().GetRegistry()));
    RegisterMarketDataRegistryServices(
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

#if 0
  template<typename B>
  void MarketDataClient<B>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_orderImbalancePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_orderImbalancePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryBboQuotes(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_bboQuotePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryBboQuotes(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_bboQuotePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_bookQuotePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_bookQuotePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedMarketQuote> queue) {
    m_marketQuotePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<MarketQuote> queue) {
    m_marketQuotePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_timeAndSalePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void MarketDataClient<B>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_timeAndSalePublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  SecuritySnapshot MarketDataClient<B>::LoadSecuritySnapshot(
      const Security& security) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadSecuritySnapshotService>(
        security);
    }, "Failed to load security snapshot: " +
      boost::lexical_cast<std::string>(security));
  }

  template<typename B>
  SecurityTechnicals MarketDataClient<B>::LoadSecurityTechnicals(
      const Security& security) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadSecurityTechnicalsService>(
        security);
    }, "Failed to load security technicals: " +
      boost::lexical_cast<std::string>(security));
  }

  template<typename B>
  std::vector<SecurityInfo> MarketDataClient<B>::QuerySecurityInfo(
      const SecurityInfoQuery& query) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<QuerySecurityInfoService>(query);
    }, "Failed to query for security info records: " +
      boost::lexical_cast<std::string>(query));
  }

  template<typename B>
  std::vector<SecurityInfo> MarketDataClient<B>::LoadSecurityInfoFromPrefix(
      const std::string& prefix) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadSecurityInfoFromPrefixService>(
        prefix);
    }, "Failed to load security info from prefix: \"" + prefix + "\"");
  }

  template<typename B>
  void MarketDataClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_orderImbalancePublisher.Break();
    m_bboQuotePublisher.Break();
    m_bookQuotePublisher.Break();
    m_marketQuotePublisher.Break();
    m_timeAndSalePublisher.Break();
    m_openState.Close();
  }

  template<typename B>
  void MarketDataClient<B>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_orderImbalancePublisher.Recover(*client);
    m_bboQuotePublisher.Recover(*client);
    m_bookQuotePublisher.Recover(*client);
    m_marketQuotePublisher.Recover(*client);
    m_timeAndSalePublisher.Recover(*client);
  }
#endif
}

#endif
