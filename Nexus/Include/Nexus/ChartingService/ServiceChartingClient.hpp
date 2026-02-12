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
#include "Nexus/ChartingService/TickerChartingQuery.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus {

  /**
   * Implements a ChartingClient using Beam services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceChartingClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::dereference_t<B>;

      /**
       * Constructs a ServiceChartingClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ServiceChartingClient(BF&& client_builder);

      ~ServiceChartingClient();

      void query(const TickerChartingQuery& query,
        Beam::ScopedQueueWriter<QueryVariant> queue);
      PriceQueryResult load_price_series(const Ticker& ticker,
        boost::posix_time::ptime start, boost::posix_time::ptime end,
        boost::posix_time::time_duration interval);
      void close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      using TickerChartingPublisher =
        Beam::SequencedValuePublisher<TickerChartingQuery, QueryVariant>;
      boost::atomic_int m_next_query_id;
      Beam::SynchronizedUnorderedMap<
        int, std::shared_ptr<TickerChartingPublisher>>
          m_ticker_charting_publishers;
      Beam::ServiceProtocolClientHandler<B> m_client_handler;
      Beam::OpenState m_open_state;
      Beam::RoutineHandlerGroup m_query_routines;

      ServiceChartingClient(const ServiceChartingClient&) = delete;
      ServiceChartingClient& operator =(const ServiceChartingClient&) = delete;
      void on_ticker_query(ServiceProtocolClient& client, int query_id,
        const SequencedQueryVariant& value);
  };

  template<typename B>
  template<typename BF>
  ServiceChartingClient<B>::ServiceChartingClient(BF&& client_builder)
      try : m_next_query_id(0),
            m_client_handler(std::forward<BF>(client_builder)) {
    Nexus::register_query_types(
      Beam::out(m_client_handler.get_slots().get_registry()));
    register_charting_services(out(m_client_handler.get_slots()));
    register_charting_messages(out(m_client_handler.get_slots()));
    Beam::add_message_slot<TickerQueryMessage>(
      out(m_client_handler.get_slots()),
      std::bind_front(&ServiceChartingClient::on_ticker_query, this));
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to connect to the charting server."));
  }

  template<typename B>
  ServiceChartingClient<B>::~ServiceChartingClient() {
    close();
  }

  template<typename B>
  void ServiceChartingClient<B>::query(const TickerChartingQuery& query,
      Beam::ScopedQueueWriter<QueryVariant> queue) {
    if(query.get_range().get_end() == Beam::Sequence::LAST) {
      m_query_routines.spawn([=, this, queue = std::move(queue)] () mutable {
        auto filter = Beam::translate<EvaluatorTranslator>(query.get_filter());
        auto conversion_queue = Beam::convert<SequencedQueryVariant>(
          std::move(queue), [] (const auto& value) {
            return *value;
          });
        auto publisher = std::make_shared<TickerChartingPublisher>(
          query, std::move(filter), std::move(conversion_queue));
        auto id = ++m_next_query_id;
        try {
          publisher->begin_snapshot();
          m_ticker_charting_publishers.insert(id, publisher);
          auto client = m_client_handler.get_client();
          auto query_result =
            client->template send_request<QueryTickerService>(query, id);
          publisher->push_snapshot(
            query_result.m_snapshot.begin(), query_result.m_snapshot.end());
          publisher->end_snapshot(query_result.m_id);
        } catch(const std::exception&) {
          publisher->close();
          m_ticker_charting_publishers.erase(id);
        }
      });
    } else {
      m_query_routines.spawn([=, this, queue = std::move(queue)] () mutable {
        try {
          auto client = m_client_handler.get_client();
          auto id = ++m_next_query_id;
          auto query_result =
            client->template send_request<QueryTickerService>(query, id);
          for(auto& value : query_result.m_snapshot) {
            queue.push(std::move(value));
          }
          queue.close();
        } catch(const std::exception&) {
          queue.close(std::current_exception());
        }
      });
    }
  }

  template<typename B>
  PriceQueryResult ServiceChartingClient<B>::load_price_series(
      const Ticker& ticker, boost::posix_time::ptime start,
      boost::posix_time::ptime end, boost::posix_time::time_duration interval) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadTickerPriceSeriesService>(
        ticker, start, end, interval);
    }, "Failed to load price series: " +
      boost::lexical_cast<std::string>(ticker) + ", " +
      boost::lexical_cast<std::string>(start) + ", " +
      boost::lexical_cast<std::string>(end) + ", " +
      boost::lexical_cast<std::string>(interval));
  }

  template<typename B>
  void ServiceChartingClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_client_handler.close();
    m_ticker_charting_publishers.clear();
    m_open_state.close();
  }

  template<typename B>
  void ServiceChartingClient<B>::on_ticker_query(ServiceProtocolClient& client,
      int query_id, const SequencedQueryVariant& value) {
    auto check_publisher = m_ticker_charting_publishers.try_load(query_id);
    if(!check_publisher) {
      return;
    }
    auto& publisher = *check_publisher;
    try {
      publisher->push(value);
    } catch(const std::exception&) {
      if(publisher->get_id() != -1) {
        auto client = m_client_handler.get_client();
        Beam::send_record_message<EndTickerQueryMessage>(*client, query_id);
      }
    }
  }
}

#endif
