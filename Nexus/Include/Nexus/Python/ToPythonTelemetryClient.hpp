#ifndef NEXUS_PYTHON_TELEMETRY_CLIENT_HPP
#define NEXUS_PYTHON_TELEMETRY_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/TelemetryService/TelemetryClientBox.hpp"

namespace Nexus::TelemetryService {

  /**
   * Wraps a TelemetryClient for use with Python.
   * param <C> The type of TelemetryClient to wrap.
   */
  template<typename C>
  class ToPythonTelemetryClient {
    public:

      /** The type of TelemetryClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonTelemetryClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonTelemetryClient, Args...>>
      ToPythonTelemetryClient(Args&&... args);

      ~ToPythonTelemetryClient();

      /** Returns the wrapped client. */
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

      void QueryTelemetryEvents(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue);

      void QueryTelemetryEvents(const AccountQuery& query,
        Beam::ScopedQueueWriter<TelemetryEvent> queue);

      void Record(const std::string& name, const Beam::JsonObject& data);

      void Close();

    private:
      boost::optional<Client> m_client;

      ToPythonTelemetryClient(const ToPythonTelemetryClient&) = delete;
      ToPythonTelemetryClient& operator =(const ToPythonTelemetryClient&) =
        delete;
  };

  template<typename Client>
  ToPythonTelemetryClient(Client&&) ->
    ToPythonTelemetryClient<std::decay_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonTelemetryClient<C>::ToPythonTelemetryClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonTelemetryClient<C>::~ToPythonTelemetryClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonTelemetryClient<C>::Client&
      ToPythonTelemetryClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonTelemetryClient<C>::Client&
      ToPythonTelemetryClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  void ToPythonTelemetryClient<C>::QueryTelemetryEvents(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedTelemetryEvent> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryTelemetryEvents(query, std::move(queue));
  }

  template<typename C>
  void ToPythonTelemetryClient<C>::QueryTelemetryEvents(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<TelemetryEvent> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryTelemetryEvents(query, std::move(queue));
  }

  template<typename C>
  void ToPythonTelemetryClient<C>::Record(
      const std::string& name, const Beam::JsonObject& data) {
    auto release = Beam::Python::GilRelease();
    m_client->Record(name, data);
  }

  template<typename C>
  void ToPythonTelemetryClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
