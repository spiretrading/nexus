#ifndef NEXUS_CHARTING_CLIENT_HPP
#define NEXUS_CHARTING_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus::ChartingService {

  /** Provides a generic interface over an arbitrary ChartingClient. */
  class ChartingClient {
    public:

      /**
       * Constructs a ChartingClient of a specified type using emplacement.
       * @param <T> The type of charting client to emplace.
       * @param args The arguments to pass to the emplaced charting client.
       */
      template<typename T, typename... Args>
      explicit ChartingClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ChartingClient by copying an existing charting client.
       * @param client The client to copy.
       */
      template<typename C>
      explicit ChartingClient(C client);

      explicit ChartingClient(ChartingClient* client);

      explicit ChartingClient(
        const std::shared_ptr<ChartingClient>& client);

      explicit ChartingClient(const std::unique_ptr<ChartingClient>& client);

      /**
       * Submits a query for a Security's technical info.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue);

      /**
       * Loads a Security's time/price series.
       * @param security The Security to load the series for.
       * @param start The series start time (inclusive).
       * @param end The series end time (inclusive).
       * @param interval The time interval per Candlestick.
       * @return The Security's time/price series with the specified parameters.
       */
      TimePriceQueryResult load_time_price_series(const Security& security,
        boost::posix_time::ptime start, boost::posix_time::ptime end,
        boost::posix_time::time_duration interval);

      void close();

    private:
      struct VirtualChartingClient {
        virtual ~VirtualChartingClient() = default;
        virtual void query(const SecurityChartingQuery& query,
          Beam::ScopedQueueWriter<Queries::QueryVariant> queue) = 0;
        virtual TimePriceQueryResult load_time_price_series(
          const Security& security, boost::posix_time::ptime start,
          boost::posix_time::ptime end,
          boost::posix_time::time_duration interval) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedChartingClient final : VirtualChartingClient {
        using ChartingClient = C;
        Beam::GetOptionalLocalPtr<ChartingClient> m_client;

        template<typename... Args>
        WrappedChartingClient(Args&&... args);
        void query(const SecurityChartingQuery& query,
          Beam::ScopedQueueWriter<Queries::QueryVariant> queue) override;
        TimePriceQueryResult load_time_price_series(const Security& security,
          boost::posix_time::ptime start, boost::posix_time::ptime end,
          boost::posix_time::time_duration interval) override;
        void close() override;
      };
      std::shared_ptr<VirtualChartingClient> m_client;
  };

  /** Checks if a type implements a ChartingClient. */
  template<typename T>
  concept IsChartingClient = std::constructible_from<
    ChartingClient, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  ChartingClient::ChartingClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedChartingClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename C>
  ChartingClient::ChartingClient(C client)
    : ChartingClient(std::in_place_type<C>, std::move(client)) {}

  inline ChartingClient::ChartingClient(ChartingClient* client)
    : ChartingClient(*client) {}

  inline ChartingClient::ChartingClient(
    const std::shared_ptr<ChartingClient>& client)
    : ChartingClient(*client) {}

  inline ChartingClient::ChartingClient(
    const std::unique_ptr<ChartingClient>& client)
    : ChartingClient(*client) {}

  inline void ChartingClient::query(const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    m_client->query(query, std::move(queue));
  }

  inline TimePriceQueryResult ChartingClient::load_time_price_series(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, boost::posix_time::time_duration interval) {
    return m_client->load_time_price_series(security, start, end, interval);
  }

  inline void ChartingClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  ChartingClient::WrappedChartingClient<C>::WrappedChartingClient(
    Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void ChartingClient::WrappedChartingClient<C>::query(
      const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  TimePriceQueryResult ChartingClient::WrappedChartingClient<C>::
      load_time_price_series(const Security& security,
        boost::posix_time::ptime start, boost::posix_time::ptime end,
        boost::posix_time::time_duration interval) {
    return m_client->LoadTimePriceSeries(security, start, end, interval);
  }

  template<typename C>
  void ChartingClient::WrappedChartingClient<C>::close() {
    m_client->Close();
  }
}

#endif
