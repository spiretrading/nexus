#ifndef NEXUS_VIRTUAL_CHARTING_CLIENT_HPP
#define NEXUS_VIRTUAL_CHARTING_CLIENT_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/ChartingService/ChartingQueryResults.hpp"
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus::ChartingService {

  /** Provides a pure virtual interface to a ChartingClient. */
  class VirtualChartingClient : private boost::noncopyable {
    public:
      virtual ~VirtualChartingClient() = default;

      virtual void QuerySecurity(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue) = 0;

      virtual TimePriceQueryResult LoadTimePriceSeries(const Security& security,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
        boost::posix_time::time_duration interval) = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualChartingClient. */
      VirtualChartingClient() = default;
  };

  /**
   * Wraps a ChartingClient providing it with a virtual interface.
   * @param <C> The type of ChartingClient to wrap.
   */
  template<typename C>
  class WrapperChartingClient : public VirtualChartingClient {
    public:

      /** The ChartingClient to wrap. */
      using Client = typename Beam::TryDereferenceType<C>::type;

      /**
       * Constructs a WrapperChartingClient.
       * @param client The ChartingClient to wrap.
       */
      template<typename CF>
      explicit WrapperChartingClient(CF&& client);

      void QuerySecurity(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue) override;

      TimePriceQueryResult LoadTimePriceSeries(const Security& security,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
        boost::posix_time::time_duration interval) override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<C> m_client;
  };

  /**
   * Wraps a ChartingClient into a VirtualChartingClient.
   * @param client The client to wrap.
   */
  template<typename ChartingClient>
  std::unique_ptr<VirtualChartingClient> MakeVirtualChartingClient(
      ChartingClient&& client) {
    return std::make_unique<WrapperChartingClient<ChartingClient>>(
      std::forward<ChartingClient>(client));
  }

  template<typename C>
  template<typename CF>
  WrapperChartingClient<C>::WrapperChartingClient(CF&& client)
    : m_client(std::forward<CF>(client)) {}

  template<typename C>
  void WrapperChartingClient<C>::QuerySecurity(
      const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    m_client->QuerySecurity(query, std::move(queue));
  }

  template<typename C>
  TimePriceQueryResult WrapperChartingClient<C>::LoadTimePriceSeries(
      const Security& security, boost::posix_time::ptime startTime,
      boost::posix_time::ptime endTime,
      boost::posix_time::time_duration interval) {
    return m_client->LoadTimePriceSeries(security, startTime, endTime,
      interval);
  }

  template<typename C>
  void WrapperChartingClient<C>::Close() {
    m_client->Close();
  }
}

#endif
