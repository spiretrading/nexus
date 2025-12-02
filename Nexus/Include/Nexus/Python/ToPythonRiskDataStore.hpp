#ifndef NEXUS_PYTHON_RISK_DATA_STORE_HPP
#define NEXUS_PYTHON_RISK_DATA_STORE_HPP
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus {

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
       * Constructs a ToPythonRiskDataStore in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonRiskDataStore(Args&&... args);

      ~ToPythonRiskDataStore();

      /** Returns a reference to the underlying data store. */
      DataStore& get();

      /** Returns a reference to the underlying data store. */
      const DataStore& get() const;

      /** Returns a reference to the underlying data store. */
      DataStore& operator *();

      /** Returns a reference to the underlying data store. */
      const DataStore& operator *() const;

      /** Returns a pointer to the underlying data store. */
      DataStore* operator ->();

      /** Returns a pointer to the underlying data store. */
      const DataStore* operator ->() const;

      InventorySnapshot load_inventory_snapshot(
        const Beam::DirectoryEntry& account);
      void store(const Beam::DirectoryEntry& account,
        const InventorySnapshot& snapshot);
      void close();

    private:
      boost::optional<DataStore> m_data_store;
  };

  template<typename DataStore>
  ToPythonRiskDataStore(DataStore&&) ->
    ToPythonRiskDataStore<std::remove_cvref_t<DataStore>>;

  template<IsRiskDataStore D>
  template<typename... Args>
  ToPythonRiskDataStore<D>::ToPythonRiskDataStore(Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsRiskDataStore D>
  ToPythonRiskDataStore<D>::~ToPythonRiskDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<IsRiskDataStore D>
  typename ToPythonRiskDataStore<D>::DataStore&
      ToPythonRiskDataStore<D>::get() {
    return *m_data_store;
  }

  template<IsRiskDataStore D>
  const typename ToPythonRiskDataStore<D>::DataStore&
      ToPythonRiskDataStore<D>::get() const {
    return *m_data_store;
  }

  template<IsRiskDataStore D>
  typename ToPythonRiskDataStore<D>::DataStore&
      ToPythonRiskDataStore<D>::operator *() {
    return *m_data_store;
  }

  template<IsRiskDataStore D>
  const typename ToPythonRiskDataStore<D>::DataStore&
      ToPythonRiskDataStore<D>::operator *() const {
    return *m_data_store;
  }

  template<IsRiskDataStore D>
  typename ToPythonRiskDataStore<D>::DataStore*
      ToPythonRiskDataStore<D>::operator ->() {
    return m_data_store.get_ptr();
  }

  template<IsRiskDataStore D>
  const typename ToPythonRiskDataStore<D>::DataStore*
      ToPythonRiskDataStore<D>::operator ->() const {
    return m_data_store.get_ptr();
  }

  template<IsRiskDataStore D>
  InventorySnapshot ToPythonRiskDataStore<D>::load_inventory_snapshot(
      const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_inventory_snapshot(account);
  }

  template<IsRiskDataStore D>
  void ToPythonRiskDataStore<D>::store(
      const Beam::DirectoryEntry& account,
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
