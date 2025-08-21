#ifndef NEXUS_TO_PYTHON_CHARTING_CLIENT_HPP
#define NEXUS_TO_PYTHON_CHARTING_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/ChartingService/ChartingClient.hpp"

namespace Nexus::ChartingService {

  /**
   * Wraps an ChartingClient for use with Python.
   * @tparam <C> The type of ChartingClient to wrap.
   */
  template<IsChartingClient C>
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
      const Client& get_client() const;

      /** Returns the wrapped client. */
      Client& get_client();
      void query(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue);
      TimePriceQueryResult load_time_price_series(const Security& security,
        boost::posix_time::ptime start, boost::posix_time::ptime end,
        boost::posix_time::time_duration interval);
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonChartingClient(const ToPythonChartingClient&) = delete;
      ToPythonChartingClient& operator =(
        const ToPythonChartingClient&) = delete;
  };

  template<typename Client>
  ToPythonChartingClient(Client&&) ->
    ToPythonChartingClient<std::remove_reference_t<Client>>;

  template<IsChartingClient C>
  template<typename... Args, typename>
  ToPythonChartingClient<C>::ToPythonChartingClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsChartingClient C>
  ToPythonChartingClient<C>::~ToPythonChartingClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsChartingClient C>
  const typename ToPythonChartingClient<C>::Client&
      ToPythonChartingClient<C>::get_client() const {
    return *m_client;
  }

  template<IsChartingClient C>
  typename ToPythonChartingClient<C>::Client&
      ToPythonChartingClient<C>::get_client() {
    return *m_client;
  }

  template<IsChartingClient C>
  void ToPythonChartingClient<C>::query(const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsChartingClient C>
  TimePriceQueryResult ToPythonChartingClient<C>::load_time_price_series(
      const Security& security, boost::posix_time::ptime start,
      boost::posix_time::ptime end, boost::posix_time::time_duration interval) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_time_price_series(security, start, end, interval);
  }

  template<IsChartingClient C>
  void ToPythonChartingClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
