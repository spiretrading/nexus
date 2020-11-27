#ifndef NEXUS_MARKET_DATA_CLIENT_HPP
#define NEXUS_MARKET_DATA_CLIENT_HPP
#include <vector>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/QueryClientPublisher.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/lexical_cast.hpp>
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
  class MarketDataClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a MarketDataClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit MarketDataClient(BF&& clientBuilder);

      ~MarketDataClient();

      /**
       * Submits a query for a Market's OrderImbalances.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);

      /**
       * Submits a query for a Market's OrderImbalances.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);

      /**
       * Submits a query for a Security's BboQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);

      /**
       * Submits a query for a Security's BboQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);

      /**
       * Submits a query for a Security's BookQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);

      /**
       * Submits a query for a Security's BookQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);

      /**
       * Submits a query for a Security's MarketQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue);

      /**
       * Submits a query for a Security's MarketQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue);

      /**
       * Submits a query for a Security's TimeAndSales.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);

      /**
       * Submits a query for a Security's TimeAndSales.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);

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

      MarketDataClient(const MarketDataClient&) = delete;
      MarketDataClient& operator =(const MarketDataClient&) = delete;
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
  };

  template<typename B>
  template<typename BF>
  MarketDataClient<B>::MarketDataClient(BF&& clientBuilder)
      try : m_clientHandler(std::forward<BF>(clientBuilder),
              std::bind(&MarketDataClient::OnReconnect, this,
              std::placeholders::_1)),
            m_orderImbalancePublisher(Beam::Ref(m_clientHandler)),
            m_bboQuotePublisher(Beam::Ref(m_clientHandler)),
            m_bookQuotePublisher(Beam::Ref(m_clientHandler)),
            m_marketQuotePublisher(Beam::Ref(m_clientHandler)),
            m_timeAndSalePublisher(Beam::Ref(m_clientHandler)) {
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
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the market data server."));
  }

  template<typename B>
  MarketDataClient<B>::~MarketDataClient() {
    Close();
  }

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
  boost::optional<SecurityInfo> MarketDataClient<B>::LoadSecurityInfo(
      const Security& security) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadSecurityInfoService>(security);
    }, "Failed to load security info: " +
      boost::lexical_cast<std::string>(security));
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
}

#endif
