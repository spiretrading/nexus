#ifndef NEXUS_SERVICE_CHARTING_CLIENT_HPP
#define NEXUS_SERVICE_CHARTING_CLIENT_HPP
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
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus::ChartingService {

  /**
   * Implements a ChartingClient using Beam services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceChartingClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a ServiceChartingClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ServiceChartingClient(BF&& client_builder);
      ~ServiceChartingClient();
      void query(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue);
      TimePriceQueryResult load_time_price_series(const Security& security,
        boost::posix_time::ptime start, boost::posix_time::ptime end,
        boost::posix_time::time_duration interval);
      void close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      using SecurityChartingPublisher = Beam::Queries::SequencedValuePublisher<
        SecurityChartingQuery, Queries::QueryVariant>;
      boost::atomic_int m_next_query_id;
      Beam::SynchronizedUnorderedMap<
        int, std::shared_ptr<SecurityChartingPublisher>>
          m_security_charting_publishers;
      Beam::Services::ServiceProtocolClientHandler<B> m_client_handler;
      Beam::IO::OpenState m_open_state;
      Beam::Routines::RoutineHandlerGroup m_query_routines;

      ServiceChartingClient(const ServiceChartingClient&) = delete;
      ServiceChartingClient& operator =(const ServiceChartingClient&) = delete;
      void on_security_query(ServiceProtocolClient& client, int query_id,
        const Queries::SequencedQueryVariant& value);
  };

  template<typename B>
  template<typename BF>
  ServiceChartingClient<B>::ServiceChartingClient(BF&& client_builder)
      try : m_next_query_id(0),
            m_client_handler(std::forward<BF>(client_builder)) {
    Queries::RegisterQueryTypes(Beam::Store(
      m_client_handler.GetSlots().GetRegistry()));
    RegisterChartingServices(Store(m_client_handler.GetSlots()));
    RegisterChartingMessages(Store(m_client_handler.GetSlots()));
    Beam::Services::AddMessageSlot<SecurityQueryMessage>(
      Store(m_client_handler.GetSlots()),
      std::bind_front(&ServiceChartingClient::on_security_query, this));
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the charting server."));
  }

  template<typename B>
  ServiceChartingClient<B>::~ServiceChartingClient() {
    close();
  }

  template<typename B>
  void ServiceChartingClient<B>::query(const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      m_query_routines.Spawn([=, this, queue = std::move(queue)] () mutable {
        auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
          query.GetFilter());
        auto conversion_queue =
          Beam::MakeConverterQueueWriter<Queries::SequencedQueryVariant>(
            std::move(queue), [] (const auto& value) {
              return *value;
            });
        auto publisher = std::make_shared<SecurityChartingPublisher>(
          query, std::move(filter), std::move(conversion_queue));
        auto id = ++m_next_query_id;
        try {
          publisher->BeginSnapshot();
          m_security_charting_publishers.Insert(id, publisher);
          auto client = m_client_handler.GetClient();
          auto query_result =
            client->template SendRequest<QuerySecurityService>(query, id);
          publisher->PushSnapshot(
            query_result.m_snapshot.begin(), query_result.m_snapshot.end());
          publisher->EndSnapshot(query_result.m_queryId);
        } catch(const std::exception&) {
          publisher->Break();
          m_security_charting_publishers.Erase(id);
        }
      });
    } else {
      m_query_routines.Spawn([=, this, queue = std::move(queue)] () mutable {
        try {
          auto client = m_client_handler.GetClient();
          auto id = ++m_next_query_id;
          auto query_result =
            client->template SendRequest<QuerySecurityService>(query, id);
          for(auto& value : query_result.m_snapshot) {
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
  TimePriceQueryResult ServiceChartingClient<B>::load_time_price_series(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, boost::posix_time::time_duration interval) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadSecurityTimePriceSeriesService>(
        security, start, end, interval);
    }, "Failed to load time price series: " +
      boost::lexical_cast<std::string>(security) + ", " +
      boost::lexical_cast<std::string>(start) + ", " +
      boost::lexical_cast<std::string>(end) + ", " +
      boost::lexical_cast<std::string>(interval));
  }

  template<typename B>
  void ServiceChartingClient<B>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_client_handler.Close();
    m_security_charting_publishers.Clear();
    m_open_state.Close();
  }

  template<typename B>
  void ServiceChartingClient<B>::on_security_query(
      ServiceProtocolClient& client, int query_id,
      const Queries::SequencedQueryVariant& value) {
    auto check_publisher = m_security_charting_publishers.FindValue(query_id);
    if(!check_publisher) {
      return;
    }
    auto& publisher = *check_publisher;
    try {
      publisher->Push(value);
    } catch(const std::exception&) {
      if(publisher->GetId() != -1) {
        auto client = m_client_handler.GetClient();
        Beam::Services::SendRecordMessage<EndSecurityQueryMessage>(
          *client, query_id);
      }
    }
  }
}

#endif
