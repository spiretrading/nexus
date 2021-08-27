#ifndef NEXUS_CHARTING_CLIENT_HPP
#define NEXUS_CHARTING_CLIENT_HPP
#include <vector>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SequencedValuePublisher.hpp>
#include <Beam/Queues/ConverterQueueWriter.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus::ChartingService {

  /**
   * Client used to access the charting related services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ChartingClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a ChartingClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ChartingClient(BF&& clientBuilder);

      ~ChartingClient();

      /**
       * Submits a query for a Security's technical info.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QuerySecurity(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue);

      /**
       * Loads a Security's time/price series.
       * @param security The Security to load the series for.
       * @param startTime The series start time (inclusive).
       * @param endTime The series end time (inclusive).
       * @param interval The time interval per Candlestick.
       * @return The Security's time/price series with the specified parameters.
       */
      TimePriceQueryResult LoadTimePriceSeries(const Security& security,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
        boost::posix_time::time_duration interval);

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      using SecurityChartingPublisher = Beam::Queries::SequencedValuePublisher<
        SecurityChartingQuery, Queries::QueryVariant>;
      boost::atomic_int m_nextQueryId;
      Beam::SynchronizedUnorderedMap<int,
        std::shared_ptr<SecurityChartingPublisher>>
        m_securityChartingPublishers;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::IO::OpenState m_openState;
      Beam::Routines::RoutineHandlerGroup m_queryRoutines;

      ChartingClient(const ChartingClient&) = delete;
      ChartingClient& operator =(const ChartingClient&) = delete;
      void OnSecurityQuery(ServiceProtocolClient& client, int queryId,
        const Queries::SequencedQueryVariant& value);
  };

  template<typename B>
  template<typename BF>
  ChartingClient<B>::ChartingClient(BF&& clientBuilder)
      try : m_nextQueryId(0),
            m_clientHandler(std::forward<BF>(clientBuilder)) {
    Queries::RegisterQueryTypes(Beam::Store(
      m_clientHandler.GetSlots().GetRegistry()));
    RegisterChartingServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterChartingMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<SecurityQueryMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind_front(&ChartingClient::OnSecurityQuery, this));
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the charting server."));
  }

  template<typename B>
  ChartingClient<B>::~ChartingClient() {
    Close();
  }

  template<typename B>
  void ChartingClient<B>::QuerySecurity(const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      m_queryRoutines.Spawn([=, this, queue = std::move(queue)] () mutable {
        auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
          query.GetFilter());
        auto conversionQueue = Beam::MakeConverterQueueWriter<
          Queries::SequencedQueryVariant>(std::move(queue),
          [] (const auto& value) {
            return *value;
          });
        auto publisher = std::make_shared<SecurityChartingPublisher>(query,
          std::move(filter), std::move(conversionQueue));
        auto id = ++m_nextQueryId;
        try {
          publisher->BeginSnapshot();
          m_securityChartingPublishers.Insert(id, publisher);
          auto client = m_clientHandler.GetClient();
          auto queryResult = client->template SendRequest<QuerySecurityService>(
            query, id);
          publisher->PushSnapshot(queryResult.m_snapshot.begin(),
            queryResult.m_snapshot.end());
          publisher->EndSnapshot(queryResult.m_queryId);
        } catch(const std::exception&) {
          publisher->Break();
          m_securityChartingPublishers.Erase(id);
        }
      });
    } else {
      m_queryRoutines.Spawn([=, this, queue = std::move(queue)] () mutable {
        try {
          auto client = m_clientHandler.GetClient();
          auto id = ++m_nextQueryId;
          auto queryResult = client->template SendRequest<QuerySecurityService>(
            query, id);
          for(auto& value : queryResult.m_snapshot) {
            queue.Push(std::move(value));
          }
          queue.Break();
        } catch(const std::exception&) {
          queue.Break(std::current_exception());
        }
      });
    }
  }

  template<typename B>
  TimePriceQueryResult ChartingClient<B>::LoadTimePriceSeries(
      const Security& security, boost::posix_time::ptime startTime,
      boost::posix_time::ptime endTime,
      boost::posix_time::time_duration interval) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadSecurityTimePriceSeriesService>(
        security, startTime, endTime, interval);
    }, "Failed to load time price series: " +
      boost::lexical_cast<std::string>(security) + ", " +
      boost::lexical_cast<std::string>(startTime) + ", " +
      boost::lexical_cast<std::string>(endTime) + ", " +
      boost::lexical_cast<std::string>(interval));
  }

  template<typename B>
  void ChartingClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_securityChartingPublishers.Clear();
    m_openState.Close();
  }

  template<typename B>
  void ChartingClient<B>::OnSecurityQuery(ServiceProtocolClient& client,
      int queryId, const Queries::SequencedQueryVariant& value) {
    auto checkPublisher = m_securityChartingPublishers.FindValue(queryId);
    if(!checkPublisher) {
      return;
    }
    auto& publisher = *checkPublisher;
    try {
      publisher->Push(value);
    } catch(const std::exception&) {
      if(publisher->GetId() != -1) {
        auto client = m_clientHandler.GetClient();
        Beam::Services::SendRecordMessage<EndSecurityQueryMessage>(*client,
          queryId);
      }
    }
  }
}

#endif
