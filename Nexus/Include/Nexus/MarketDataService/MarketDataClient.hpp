#ifndef NEXUS_MARKET_DATA_CLIENT_HPP
#define NEXUS_MARKET_DATA_CLIENT_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/QueryClientPublisher.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus::MarketDataService {

  /**
   * Client used to access market data services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class MarketDataClient : private boost::noncopyable {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a MarketDataClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename ClientBuilderForward>
      MarketDataClient(ClientBuilderForward&& clientBuilder);

      ~MarketDataClient();

      /**
       * Submits a query for a Market's OrderImbalances.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedOrderImbalance>>&
        queue);

      /**
       * Submits a query for a Market's OrderImbalances.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue);

      /**
       * Submits a query for a Security's BboQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue);

      /**
       * Submits a query for a Security's BboQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue);

      /**
       * Submits a query for a Security's BookQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue);

      /**
       * Submits a query for a Security's BookQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue);

      /**
       * Submits a query for a Security's MarketQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue);

      /**
       * Submits a query for a Security's MarketQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue);

      /**
       * Submits a query for a Security's TimeAndSales.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue);

      /**
       * Submits a query for a Security's TimeAndSales.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue);

      /**
       * Loads a Security's real-time snapshot.
       * @param security The Security whose SecuritySnapshot is to be loaded.
       * @return The real-time snapshot of the specified <i>security</i>.
       */
      SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      /**
       * Loads the SecurityTechnicals for a specified Security.
       * @param security The Security whose SecurityTechnicals is to be loaded.
       * @return The SecurityTechnicals for the specified <i>security</i>.
       */
      SecurityTechnicals LoadSecurityTechnicals(const Security& security);

      /**
       * Loads the SecurityInfo for a specified Security.
       * @param security The Security whose SecurityInfo is to be loaded.
       * @return The SecurityInfo for the specified <i>security</i>.
       */
      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      /**
       * Loads SecurityInfo objects that match a prefix.
       * @param prefix The prefix to search for.
       * @return The list of SecurityInfo objects that match the <i>prefix</i>.
       */
      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Open();

      void Close();

    private:
      template<typename Value, typename Query, typename QueryService,
        typename EndQueryMessage>
      using QueryClientPublisher = Beam::Queries::QueryClientPublisher<Value,
        Query, Queries::EvaluatorTranslator,
        Beam::Services::ServiceProtocolClientHandler<B>, QueryService,
        EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      QueryClientPublisher<OrderImbalance, MarketWideDataQuery,
        QueryOrderImbalancesService, EndOrderImbalanceQueryMessage>
        m_orderImbalancePublisher;
      QueryClientPublisher<BboQuote, SecurityMarketDataQuery,
        QueryBboQuotesService, EndBboQuoteQueryMessage> m_bboQuotePublisher;
      QueryClientPublisher<BookQuote, SecurityMarketDataQuery,
        QueryBookQuotesService, EndBookQuoteQueryMessage> m_bookQuotePublisher;
      QueryClientPublisher<MarketQuote, SecurityMarketDataQuery,
        QueryMarketQuotesService, EndMarketQuoteQueryMessage>
        m_marketQuotePublisher;
      QueryClientPublisher<TimeAndSale, SecurityMarketDataQuery,
        QueryTimeAndSalesService, EndTimeAndSaleQueryMessage>
        m_timeAndSalePublisher;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
  };

  template<typename B>
  template<typename ClientBuilderForward>
  MarketDataClient<B>::MarketDataClient(ClientBuilderForward&& clientBuilder)
      : m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)),
        m_orderImbalancePublisher(Beam::Ref(m_clientHandler)),
        m_bboQuotePublisher(Beam::Ref(m_clientHandler)),
        m_bookQuotePublisher(Beam::Ref(m_clientHandler)),
        m_marketQuotePublisher(Beam::Ref(m_clientHandler)),
        m_timeAndSalePublisher(Beam::Ref(m_clientHandler)) {
    m_clientHandler.SetReconnectHandler(
      std::bind(&MarketDataClient::OnReconnect, this, std::placeholders::_1));
    Queries::RegisterQueryTypes(
      Beam::Store(m_clientHandler.GetSlots().GetRegistry()));
    RegisterMarketDataRegistryServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterMarketDataRegistryMessages(Beam::Store(m_clientHandler.GetSlots()));
    m_orderImbalancePublisher.
      template AddMessageHandler<OrderImbalanceMessage>();
    m_bboQuotePublisher.template AddMessageHandler<BboQuoteMessage>();
    m_bookQuotePublisher.template AddMessageHandler<BookQuoteMessage>();
    m_marketQuotePublisher.template AddMessageHandler<MarketQuoteMessage>();
    m_timeAndSalePublisher.template AddMessageHandler<TimeAndSaleMessage>();
  }

  template<typename B>
  MarketDataClient<B>::~MarketDataClient() {
    Close();
  }

  template<typename B>
  void MarketDataClient<B>::QueryOrderImbalances(
      const MarketWideDataQuery& query, const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    m_orderImbalancePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    m_orderImbalancePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryBboQuotes(const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    m_bboQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryBboQuotes(const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    m_bboQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    m_bookQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    m_bookQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    m_marketQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    m_marketQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    m_timeAndSalePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  void MarketDataClient<B>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    m_timeAndSalePublisher.SubmitQuery(query, queue);
  }

  template<typename B>
  SecuritySnapshot MarketDataClient<B>::LoadSecuritySnapshot(
      const Security& security) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSecuritySnapshotService>(security);
  }

  template<typename B>
  SecurityTechnicals MarketDataClient<B>::LoadSecurityTechnicals(
      const Security& security) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSecurityTechnicalsService>(
      security);
  }

  template<typename B>
  boost::optional<SecurityInfo> MarketDataClient<B>::LoadSecurityInfo(
      const Security& security) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSecurityInfoService>(security);
  }

  template<typename B>
  std::vector<SecurityInfo> MarketDataClient<B>::LoadSecurityInfoFromPrefix(
      const std::string& prefix) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSecurityInfoFromPrefixService>(
      prefix);
  }

  template<typename B>
  void MarketDataClient<B>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename B>
  void MarketDataClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename B>
  void MarketDataClient<B>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
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
}

#endif
