#ifndef NEXUS_CHARTING_CLIENT_HPP
#define NEXUS_CHARTING_CLIENT_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"

namespace Nexus {

  /** Checks if a type implements a ChartingClient. */
  template<typename T>
  concept IsChartingClient = Beam::IsConnection<T> && requires(T& client) {
    client.query(std::declval<const SecurityChartingQuery&>(),
      std::declval<Beam::ScopedQueueWriter<QueryVariant>>());
    { client.load_time_price_series(std::declval<const Security&>(),
        std::declval<boost::posix_time::ptime>(),
        std::declval<boost::posix_time::ptime>(),
        std::declval<boost::posix_time::time_duration>()) } ->
          std::same_as<TimePriceQueryResult>;
  };

  /** Provides a generic interface over an arbitrary ChartingClient. */
  class ChartingClient {
    public:

      /**
       * Constructs a ChartingClient of a specified type using emplacement.
       * @tparam T The type of charting client to emplace.
       * @param args The arguments to pass to the emplaced charting client.
       */
      template<IsChartingClient T, typename... Args>
      explicit ChartingClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ChartingClient by referencing an existing charting
       * client.
       * @param client The client to reference.
       */
      template<Beam::DisableCopy<ChartingClient> T> requires
        IsChartingClient<Beam::dereference_t<T>>
      ChartingClient(T&& client);

      ChartingClient(const ChartingClient&) = default;
      ChartingClient(ChartingClient&&) = default;

      /**
       * Submits a query for a Security's technical info.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<QueryVariant> queue);

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
          Beam::ScopedQueueWriter<QueryVariant> queue) = 0;
        virtual TimePriceQueryResult load_time_price_series(
          const Security& security, boost::posix_time::ptime start,
          boost::posix_time::ptime end,
          boost::posix_time::time_duration interval) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedChartingClient final : VirtualChartingClient {
        using ChartingClient = C;
        Beam::local_ptr_t<ChartingClient> m_client;

        template<typename... Args>
        WrappedChartingClient(Args&&... args);

        void query(const SecurityChartingQuery& query,
          Beam::ScopedQueueWriter<QueryVariant> queue) override;
        TimePriceQueryResult load_time_price_series(const Security& security,
          boost::posix_time::ptime start, boost::posix_time::ptime end,
          boost::posix_time::time_duration interval) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualChartingClient> m_client;
  };

  template<IsChartingClient T, typename... Args>
  ChartingClient::ChartingClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(Beam::make_virtual_ptr<WrappedChartingClient<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<ChartingClient> T> requires
    IsChartingClient<Beam::dereference_t<T>>
  ChartingClient::ChartingClient(T&& client)
    : m_client(Beam::make_virtual_ptr<WrappedChartingClient<
        std::remove_cvref_t<T>>>(std::forward<T>(client))) {}

  inline void ChartingClient::query(const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<QueryVariant> queue) {
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
      Beam::ScopedQueueWriter<QueryVariant> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  TimePriceQueryResult ChartingClient::WrappedChartingClient<C>::
      load_time_price_series(const Security& security,
        boost::posix_time::ptime start, boost::posix_time::ptime end,
        boost::posix_time::time_duration interval) {
    return m_client->load_time_price_series(security, start, end, interval);
  }

  template<typename C>
  void ChartingClient::WrappedChartingClient<C>::close() {
    m_client->close();
  }
}

#endif
