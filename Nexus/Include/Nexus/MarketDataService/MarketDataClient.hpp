#ifndef NEXUS_MARKETDATACLIENT_HPP
#define NEXUS_MARKETDATACLIENT_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/QueryClientPublisher.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MarketDataClient
      \brief Client used to access market data services.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class MarketDataClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<
        ServiceProtocolClientBuilderType>;

      //! Constructs a MarketDataClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      MarketDataClient(ClientBuilderForward&& clientBuilder);

      ~MarketDataClient();

      //! Submits a query for a Market's OrderImbalances.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedOrderImbalance>>&
        queue);

      //! Submits a query for a Market's OrderImbalances.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue);

      //! Submits a query for a Security's BboQuotes.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue);

      //! Submits a query for a Security's BboQuotes.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue);

      //! Submits a query for a Security's BookQuotes.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue);

      //! Submits a query for a Security's BookQuotes.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue);

      //! Submits a query for a Security's MarketQuotes.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue);

      //! Submits a query for a Security's MarketQuotes.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue);

      //! Submits a query for a Security's TimeAndSales.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue);

      //! Submits a query for a Security's TimeAndSales.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue);

      //! Loads a Security's real-time snapshot.
      /*!
        \param security The Security whose SecuritySnapshot is to be loaded.
        \return The real-time snapshot of the specified <i>security</i>.
      */
      SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      //! Loads the SecurityTechnicals for a specified Security.
      /*!
        \param security The Security whose SecurityTechnicals is to be loaded.
        \return The SecurityTechnicals for the specified <i>security</i>.
      */
      SecurityTechnicals LoadSecurityTechnicals(const Security& security);

      //! Loads SecurityInfo objects that match a prefix.
      /*!
        \param prefix The prefix to search for.
        \return The list of SecurityInfo objects that match the <i>prefix</i>.
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
        Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType>, QueryService, EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
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

  //! Submits a query for a Security's real-time BookQuotes with snapshot.
  /*!
    \param marketDataClient The MarketDataClient used to submit the query.
    \param security The Security to query for.
    \param queue The queue that will store the result of the query.
    \param interruptionPolicy The policy used when the query is interrupted.
  */
  template<typename MarketDataClient>
  Beam::Routines::Routine::Id QueryRealTimeBookQuotesWithSnapshot(
      MarketDataClient& marketDataClient, const Security& security,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue,
      Beam::Queries::InterruptionPolicy interruptionPolicy =
      Beam::Queries::InterruptionPolicy::BREAK_QUERY) {
    return Beam::Routines::Spawn(
      [&marketDataClient, security, queue, interruptionPolicy] {
        SecuritySnapshot snapshot;
        try {
          snapshot = marketDataClient.LoadSecuritySnapshot(security);
        } catch(const std::exception& e) {
          queue->Break(e);
          return;
        }
        if(snapshot.m_askBook.empty() && snapshot.m_bidBook.empty()) {
          SecurityMarketDataQuery bookQuoteQuery;
          bookQuoteQuery.SetIndex(security);
          bookQuoteQuery.SetRange(Beam::Queries::Range::RealTime());
          bookQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryBookQuotes(bookQuoteQuery, queue);
        } else {
          auto startPoint = Beam::Queries::Sequence::First();
          try {
            for(auto& bookQuote : snapshot.m_askBook) {
              startPoint = std::max(startPoint, bookQuote.GetSequence());
              queue->Push(std::move(*bookQuote));
            }
            for(auto& bookQuote : snapshot.m_bidBook) {
              startPoint = std::max(startPoint, bookQuote.GetSequence());
              queue->Push(std::move(*bookQuote));
            }
          } catch(const std::exception&) {
            return;
          }
          startPoint = Beam::Queries::Increment(startPoint);
          SecurityMarketDataQuery bookQuoteQuery;
          bookQuoteQuery.SetIndex(security);
          bookQuoteQuery.SetRange(startPoint, Beam::Queries::Sequence::Last());
          bookQuoteQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Unlimited());
          bookQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryBookQuotes(bookQuoteQuery, queue);
        }
      });
  }

  //! Submits a query for a Security's real-time MarketQuotes with snapshot.
  /*!
    \param marketDataClient The MarketDataClient used to submit the query.
    \param security The Security to query for.
    \param queue The queue that will store the result of the query.
    \param interruptionPolicy The policy used when the query is interrupted.
  */
  template<typename MarketDataClient>
  Beam::Routines::Routine::Id QueryRealTimeMarketQuotesWithSnapshot(
      MarketDataClient& marketDataClient, const Security& security,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue,
      Beam::Queries::InterruptionPolicy interruptionPolicy =
      Beam::Queries::InterruptionPolicy::IGNORE_CONTINUE) {
    return Beam::Routines::Spawn(
      [&marketDataClient, security, queue, interruptionPolicy] {
        SecuritySnapshot snapshot;
        try {
          snapshot = marketDataClient.LoadSecuritySnapshot(security);
        } catch(const std::exception& e) {
          queue->Break(e);
          return;
        }
        if(snapshot.m_marketQuotes.empty()) {
          SecurityMarketDataQuery marketQuoteQuery;
          marketQuoteQuery.SetIndex(security);
          marketQuoteQuery.SetRange(Beam::Queries::Range::RealTime());
          marketQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryMarketQuotes(marketQuoteQuery, queue);
        } else {
          auto startPoint = Beam::Queries::Sequence::First();
          try {
            for(auto& marketQuote : snapshot.m_marketQuotes |
                boost::adaptors::map_values) {
              startPoint = std::max(startPoint, marketQuote.GetSequence());
              queue->Push(std::move(*marketQuote));
            }
          } catch(const std::exception&) {
            return;
          }
          startPoint = Beam::Queries::Increment(startPoint);
          SecurityMarketDataQuery marketQuoteQuery;
          marketQuoteQuery.SetIndex(security);
          marketQuoteQuery.SetRange(startPoint,
            Beam::Queries::Sequence::Last());
          marketQuoteQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Unlimited());
          marketQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryMarketQuotes(marketQuoteQuery, queue);
        }
      });
  }

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  MarketDataClient<ServiceProtocolClientBuilderType>::MarketDataClient(
      ClientBuilderForward&& clientBuilder)
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

  template<typename ServiceProtocolClientBuilderType>
  MarketDataClient<ServiceProtocolClientBuilderType>::~MarketDataClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryOrderImbalances(
      const MarketWideDataQuery& query, const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    m_orderImbalancePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    m_orderImbalancePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    m_bboQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    m_bboQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    m_bookQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    m_bookQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    m_marketQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    m_marketQuotePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    m_timeAndSalePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    m_timeAndSalePublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  SecuritySnapshot MarketDataClient<ServiceProtocolClientBuilderType>::
      LoadSecuritySnapshot(const Security& security) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSecuritySnapshotService>(security);
  }

  template<typename ServiceProtocolClientBuilderType>
  SecurityTechnicals MarketDataClient<ServiceProtocolClientBuilderType>::
      LoadSecurityTechnicals(const Security& security) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSecurityTechnicalsService>(
      security);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<SecurityInfo> MarketDataClient<ServiceProtocolClientBuilderType>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSecurityInfoFromPrefixService>(
      prefix);
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::Open() {
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

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
  }

  template<typename ServiceProtocolClientBuilderType>
  void MarketDataClient<ServiceProtocolClientBuilderType>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_orderImbalancePublisher.Recover(*client);
    m_bboQuotePublisher.Recover(*client);
    m_bookQuotePublisher.Recover(*client);
    m_marketQuotePublisher.Recover(*client);
    m_timeAndSalePublisher.Recover(*client);
  }
}
}

#endif
