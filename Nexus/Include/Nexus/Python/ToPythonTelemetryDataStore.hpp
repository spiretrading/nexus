#ifndef NEXUS_PYTHON_TELEMETRY_DATA_STORE_HPP
#define NEXUS_PYTHON_TELEMETRY_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/TelemetryService/TelemetryDataStoreBox.hpp"

namespace Nexus::TelemetryService {

  /**
   * Wraps a TelemetryDataStore for use with Python.
   * @param <D> The type of TelemetryDataStore to wrap.
   */
  template<typename D>
  class ToPythonTelemetryDataStore {
    public:

      /** The type of data store to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonTelemetryDataStore.
       * @param args The arguments to forward to the DataStore's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonTelemetryDataStore, Args...>>
      ToPythonTelemetryDataStore(Args&&... args);

      ~ToPythonTelemetryDataStore();

      /** Returns the wrapped data store. */
      const DataStore& GetDataStore() const;

      /** Returns the wrapped data store. */
      DataStore& GetDataStore();

      std::vector<SequencedTelemetryEvent> LoadTelemetryEvents(
        const AccountQuery& query);

      void Store(const SequencedAccountTelemetryEvent& event);

      void Store(const std::vector<SequencedAccountTelemetryEvent>& events);

      void Close();

    private:
      boost::optional<DataStore> m_dataStore;

      ToPythonTelemetryDataStore(const ToPythonTelemetryDataStore&) = delete;
      ToPythonTelemetryDataStore& operator =(
        const ToPythonTelemetryDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonTelemetryDataStore(DataStore&&) ->
    ToPythonTelemetryDataStore<std::decay_t<DataStore>>;

  template<typename D>
  template<typename... Args, typename>
  ToPythonTelemetryDataStore<D>::ToPythonTelemetryDataStore(
    Args&&... args)
    : m_dataStore((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename D>
  ToPythonTelemetryDataStore<D>::~ToPythonTelemetryDataStore() {
    auto release = Beam::Python::GilRelease();
    m_dataStore.reset();
  }

  template<typename D>
  const typename ToPythonTelemetryDataStore<D>::DataStore&
      ToPythonTelemetryDataStore<D>::GetDataStore() const {
    return *m_dataStore;
  }

  template<typename D>
  typename ToPythonTelemetryDataStore<D>::DataStore&
      ToPythonTelemetryDataStore<D>::GetDataStore() {
    return *m_dataStore;
  }

  template<typename D>
  std::vector<SequencedTelemetryEvent>
      ToPythonTelemetryDataStore<D>::LoadTelemetryEvents(
        const AccountQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadTelemetryEvents(query);
  }

  template<typename D>
  void ToPythonTelemetryDataStore<D>::Store(
      const SequencedAccountTelemetryEvent& event) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(event);
  }

  template<typename D>
  void ToPythonTelemetryDataStore<D>::Store(
      const std::vector<SequencedAccountTelemetryEvent>& events) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(events);
  }

  template<typename D>
  void ToPythonTelemetryDataStore<D>::Close() {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Close();
  }
}

#endif
