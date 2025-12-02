#ifndef NEXUS_TO_PYTHON_CHARTING_CLIENT_HPP
#define NEXUS_TO_PYTHON_CHARTING_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/ChartingService/ChartingClient.hpp"

namespace Nexus {

  /**
   * Wraps an ChartingClient for use with Python.
   * @param <C> The type of ChartingClient to wrap.
   */
  template<IsChartingClient C>
  class ToPythonChartingClient {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonChartingClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonChartingClient(Args&&... args);

      ~ToPythonChartingClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      /** Returns a reference to the underlying client. */
      Client& operator *();

      /** Returns a reference to the underlying client. */
      const Client& operator *() const;

      /** Returns a pointer to the underlying client. */
      Client* operator ->();

      /** Returns a pointer to the underlying client. */
      const Client* operator ->() const;

      void query(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<QueryVariant> queue);
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
    ToPythonChartingClient<std::remove_cvref_t<Client>>;

  template<IsChartingClient C>
  template<typename... Args>
  ToPythonChartingClient<C>::ToPythonChartingClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsChartingClient C>
  ToPythonChartingClient<C>::~ToPythonChartingClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsChartingClient C>
  typename ToPythonChartingClient<C>::Client&
      ToPythonChartingClient<C>::get() {
    return *m_client;
  }

  template<IsChartingClient C>
  const typename ToPythonChartingClient<C>::Client&
      ToPythonChartingClient<C>::get() const {
    return *m_client;
  }

  template<IsChartingClient C>
  typename ToPythonChartingClient<C>::Client&
      ToPythonChartingClient<C>::operator *() {
    return *m_client;
  }

  template<IsChartingClient C>
  const typename ToPythonChartingClient<C>::Client&
      ToPythonChartingClient<C>::operator *() const {
    return *m_client;
  }

  template<IsChartingClient C>
  typename ToPythonChartingClient<C>::Client*
      ToPythonChartingClient<C>::operator ->() {
    return m_client.get_ptr();
  }

  template<IsChartingClient C>
  const typename ToPythonChartingClient<C>::Client*
      ToPythonChartingClient<C>::operator ->() const {
    return m_client.get_ptr();
  }

  template<IsChartingClient C>
  void ToPythonChartingClient<C>::query(const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<QueryVariant> queue) {
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
