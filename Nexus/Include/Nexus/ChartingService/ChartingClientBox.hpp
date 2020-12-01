#ifndef NEXUS_CHARTING_CLIENT_BOX_HPP
#define NEXUS_CHARTING_CLIENT_BOX_HPP
#include <memory>
#include <type_traits>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/ChartingService/ChartingQueryResults.hpp"
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus::ChartingService {

  /** Provides a generic interface over an arbitrary ChartingClient. */
  class ChartingClientBox {
    public:

      /**
       * Constructs an ChartingClientBox of a specified type using emplacement.
       * @param <T> The type of charting client to emplace.
       * @param args The arguments to pass to the emplaced charting client.
       */
      template<typename T, typename... Args>
      explicit ChartingClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ChartingClientBox by copying an existing charting client.
       * @param client The client to copy.
       */
      template<typename ChartingClient>
      explicit ChartingClientBox(ChartingClient client);

      explicit ChartingClientBox(ChartingClientBox* client);

      explicit ChartingClientBox(
        const std::shared_ptr<ChartingClientBox>& client);

      explicit ChartingClientBox(
        const std::unique_ptr<ChartingClientBox>& client);

      void QuerySecurity(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue);

      TimePriceQueryResult LoadTimePriceSeries(const Security& security,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
        boost::posix_time::time_duration interval);

      void Close();

    private:
      struct VirtualChartingClient {
        virtual ~VirtualChartingClient() = default;
        virtual void QuerySecurity(const SecurityChartingQuery& query,
          Beam::ScopedQueueWriter<Queries::QueryVariant> queue) = 0;
        virtual TimePriceQueryResult LoadTimePriceSeries(
          const Security& security, boost::posix_time::ptime startTime,
          boost::posix_time::ptime endTime,
          boost::posix_time::time_duration interval) = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedChartingClient final : VirtualChartingClient {
        using ChartingClient = C;
        Beam::GetOptionalLocalPtr<ChartingClient> m_client;

        template<typename... Args>
        WrappedChartingClient(Args&&... args);
        void QuerySecurity(const SecurityChartingQuery& query,
          Beam::ScopedQueueWriter<Queries::QueryVariant> queue) override;
        TimePriceQueryResult LoadTimePriceSeries(const Security& security,
          boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
          boost::posix_time::time_duration interval) override;
        void Close() override;
      };
      std::shared_ptr<VirtualChartingClient> m_client;
  };

  template<typename T, typename... Args>
  ChartingClientBox::ChartingClientBox(std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedChartingClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename ChartingClient>
  ChartingClientBox::ChartingClientBox(ChartingClient client)
    : ChartingClientBox(std::in_place_type<ChartingClient>,
        std::move(client)) {}

  inline ChartingClientBox::ChartingClientBox(ChartingClientBox* client)
    : ChartingClientBox(*client) {}

  inline ChartingClientBox::ChartingClientBox(
    const std::shared_ptr<ChartingClientBox>& client)
    : ChartingClientBox(*client) {}

  inline ChartingClientBox::ChartingClientBox(
    const std::unique_ptr<ChartingClientBox>& client)
    : ChartingClientBox(*client) {}

  inline void ChartingClientBox::QuerySecurity(
      const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    m_client->QuerySecurity(query, std::move(queue));
  }

  inline TimePriceQueryResult ChartingClientBox::LoadTimePriceSeries(
      const Security& security, boost::posix_time::ptime startTime,
      boost::posix_time::ptime endTime,
      boost::posix_time::time_duration interval) {
    return m_client->LoadTimePriceSeries(security, startTime, endTime,
      interval);
  }

  inline void ChartingClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  ChartingClientBox::WrappedChartingClient<C>::WrappedChartingClient(
    Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void ChartingClientBox::WrappedChartingClient<C>::QuerySecurity(
      const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    m_client->QuerySecurity(query, std::move(queue));
  }

  template<typename C>
  TimePriceQueryResult ChartingClientBox::WrappedChartingClient<C>::
      LoadTimePriceSeries(const Security& security,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
        boost::posix_time::time_duration interval) {
    return m_client->LoadTimePriceSeries(security, startTime, endTime,
      interval);
  }

  template<typename C>
  void ChartingClientBox::WrappedChartingClient<C>::Close() {
    m_client->Close();
  }
}

#endif
