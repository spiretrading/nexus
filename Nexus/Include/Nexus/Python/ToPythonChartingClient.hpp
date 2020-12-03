#ifndef NEXUS_TO_PYTHON_CHARTING_CLIENT_HPP
#define NEXUS_TO_PYTHON_CHARTING_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/ChartingService/ChartingClientBox.hpp"

namespace Nexus::ChartingService {

  /**
   * Wraps an ChartingClient for use with Python.
   * @tparam <C> The type of ChartingClient to wrap.
   */
  template<typename C>
  class ToPythonChartingClient {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonChartingClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonChartingClient, Args...>>
      ToPythonChartingClient(Args&&... args);

      ~ToPythonChartingClient();

      /** Returns the wrapped client. */
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

      void QuerySecurity(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue);

      TimePriceQueryResult LoadTimePriceSeries(const Security& security,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
        boost::posix_time::time_duration interval);

      void Close();

    private:
      boost::optional<Client> m_client;

      ToPythonChartingClient(const ToPythonChartingClient&) = delete;
      ToPythonChartingClient& operator =(
        const ToPythonChartingClient&) = delete;
  };

  template<typename Client>
  ToPythonChartingClient(Client&&) ->
    ToPythonChartingClient<std::decay_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonChartingClient<C>::ToPythonChartingClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonChartingClient<C>::~ToPythonChartingClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonChartingClient<C>::Client&
      ToPythonChartingClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonChartingClient<C>::Client&
      ToPythonChartingClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  void ToPythonChartingClient<C>::QuerySecurity(
      const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QuerySecurity(query, std::move(queue));
  }

  template<typename C>
  TimePriceQueryResult ToPythonChartingClient<C>::LoadTimePriceSeries(
      const Security& security,
      boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
      boost::posix_time::time_duration interval) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadTimePriceSeries(security, startTime, endTime,
      interval);
  }

  template<typename C>
  void ToPythonChartingClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
