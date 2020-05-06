#ifndef NEXUS_VIRTUALCHARTINGCLIENT_HPP
#define NEXUS_VIRTUALCHARTINGCLIENT_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queues.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/ChartingService/ChartingQueryResults.hpp"
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus {
namespace ChartingService {

  /*! \class VirtualChartingClient
      \brief Provides a pure virtual interface to a ChartingClient.
   */
  class VirtualChartingClient : private boost::noncopyable {
    public:
      virtual ~VirtualChartingClient();

      virtual void QuerySecurity(const SecurityChartingQuery& query,
        const std::shared_ptr<Beam::QueueWriter<Queries::QueryVariant>>&
        queue) = 0;

      virtual TimePriceQueryResult LoadTimePriceSeries(const Security& security,
        const boost::posix_time::ptime& startTime,
        const boost::posix_time::ptime& endTime,
        boost::posix_time::time_duration interval) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualChartingClient.
      VirtualChartingClient();
  };

  /*! \class WrapperChartingClient
      \brief Wraps a ChartingClient providing it with a virtual interface.
      \tparam ClientType The type of ChartingClient to wrap.
   */
  template<typename ClientType>
  class WrapperChartingClient : public VirtualChartingClient {
    public:

      //! The ChartingClient to wrap.
      using Client = typename Beam::TryDereferenceType<ClientType>::type;

      //! Constructs a WrapperChartingClient.
      /*!
        \param client The ChartingClient to wrap.
      */
      template<typename ChartingClientForward>
      WrapperChartingClient(ChartingClientForward&& client);

      virtual ~WrapperChartingClient();

      virtual void QuerySecurity(const SecurityChartingQuery& query,
        const std::shared_ptr<Beam::QueueWriter<Queries::QueryVariant>>& queue);

      virtual TimePriceQueryResult LoadTimePriceSeries(const Security& security,
        const boost::posix_time::ptime& startTime,
        const boost::posix_time::ptime& endTime,
        boost::posix_time::time_duration interval);

      virtual void Open();

      virtual void Close();

    private:
      typename Beam::OptionalLocalPtr<ClientType>::type m_client;
  };

  //! Wraps a ChartingClient into a VirtualChartingClient.
  /*!
    \param client The client to wrap.
  */
  template<typename ChartingClient>
  std::unique_ptr<VirtualChartingClient> MakeVirtualChartingClient(
      ChartingClient&& client) {
    return std::make_unique<WrapperChartingClient<ChartingClient>>(
      std::forward<ChartingClient>(client));
  }

  inline VirtualChartingClient::~VirtualChartingClient() {}

  inline VirtualChartingClient::VirtualChartingClient() {}

  template<typename ClientType>
  template<typename ChartingClientForward>
  WrapperChartingClient<ClientType>::WrapperChartingClient(
      ChartingClientForward&& client)
      : m_client(std::forward<ChartingClientForward>(client)) {}

  template<typename ClientType>
  WrapperChartingClient<ClientType>::~WrapperChartingClient() {}

  template<typename ClientType>
  void WrapperChartingClient<ClientType>::QuerySecurity(
      const SecurityChartingQuery& query,
      const std::shared_ptr<Beam::QueueWriter<Queries::QueryVariant>>& queue) {
    m_client->QuerySecurity(query, queue);
  }

  template<typename ClientType>
  TimePriceQueryResult WrapperChartingClient<ClientType>::LoadTimePriceSeries(
      const Security& security, const boost::posix_time::ptime& startTime,
      const boost::posix_time::ptime& endTime,
      boost::posix_time::time_duration interval) {
    return m_client->LoadTimePriceSeries(security, startTime, endTime,
      interval);
  }

  template<typename ClientType>
  void WrapperChartingClient<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperChartingClient<ClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
