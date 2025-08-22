#ifndef NEXUS_PYTHON_RISK_DATA_STORE_HPP
#define NEXUS_PYTHON_RISK_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus::RiskService {

  /**
   * Wraps a RiskDataStore for use with Python.
   * @param <D> The type of RiskDataStore to wrap.
   */
  template<IsRiskDataStore D>
  class ToPythonRiskDataStore {
    public:

      /** The type of DataStore to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonRiskDataStore.
       * @param args The arguments to forward to the DataStore's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonRiskDataStore, Args...>>
      ToPythonRiskDataStore(Args&&... args);

      ~ToPythonRiskDataStore();

      /** Returns the wrapped data store. */
      const DataStore& get_data_store() const;

      /** Returns the wrapped data store. */
      DataStore& get_data_store();

      InventorySnapshot load_inventory_snapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot);
      void close();

    private:
      boost::optional<DataStore> m_data_store;
  };

  template<IsRiskDataStore D>
  template<typename... Args, typename>
  ToPythonRiskDataStore<D>::ToPythonRiskDataStore(Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsRiskDataStore D>
  ToPythonRiskDataStore<D>::~ToPythonRiskDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<IsRiskDataStore D>
  const typename ToPythonRiskDataStore<D>::DataStore&
      ToPythonRiskDataStore<D>::get_data_store() const {
    return *m_data_store;
  }

  template<IsRiskDataStore D>
  typename ToPythonRiskDataStore<D>::DataStore&
      ToPythonRiskDataStore<D>::get_data_store() {
    return *m_data_store;
  }

  template<IsRiskDataStore D>
  InventorySnapshot ToPythonRiskDataStore<D>::load_inventory_snapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_inventory_snapshot(account);
  }

  template<IsRiskDataStore D>
  void ToPythonRiskDataStore<D>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(account, snapshot);
  }

  template<IsRiskDataStore D>
  void ToPythonRiskDataStore<D>::close() {
    auto release = Beam::Python::GilRelease();
    m_data_store->close();
  }
}

#endif
