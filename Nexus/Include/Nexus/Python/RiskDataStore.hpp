#ifndef NEXUS_PYTHON_RISK_DATA_STORE_HPP
#define NEXUS_PYTHON_RISK_DATA_STORE_HPP
#include <memory>
#include <Beam/Python/GilRelease.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/RiskService/VirtualRiskDataStore.hpp"

namespace Nexus::RiskService {

  /**
   * Wraps a RiskDataStore for use with Python.
   * @param <D> The type of RiskDataStore to wrap.
   */
  template<typename D>
  class ToPythonRiskDataStore final : public VirtualRiskDataStore {
    public:

      /** The type of DataStore to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonRiskDataStore.
       * @param dataStore The data store to wrap.
       */
      ToPythonRiskDataStore(std::unique_ptr<DataStore> dataStore);

      ~ToPythonRiskDataStore() override;

      PositionSnapshot LoadPositionSnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const PositionSnapshot& snapshot) override;

      void Open() override;

      void Close() override;

    private:
      std::unique_ptr<DataStore> m_dataStore;
  };

  /**
   * Makes a ToPythonRiskDataStore.
   * @param dataStore The data store to wrap.
   */
  template<typename DataStore>
  auto MakeToPythonRiskDataStore(std::unique_ptr<DataStore> dataStore) {
    return std::make_unique<ToPythonRiskDataStore<DataStore>>(
      std::move(dataStore));
  }

  template<typename D>
  ToPythonRiskDataStore<D>::ToPythonRiskDataStore(
    std::unique_ptr<DataStore> dataStore)
    : m_dataStore(std::move(dataStore)) {}

  template<typename D>
  ToPythonRiskDataStore<D>::~ToPythonRiskDataStore() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_dataStore.reset();
  }

  template<typename D>
  PositionSnapshot ToPythonRiskDataStore<D>::LoadPositionSnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadPositionSnapshot(account);
  }

  template<typename D>
  void ToPythonRiskDataStore<D>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const PositionSnapshot& snapshot) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->Store(account, snapshot);
  }

  template<typename D>
  void ToPythonRiskDataStore<D>::Open() {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Open();
  }

  template<typename D>
  void ToPythonRiskDataStore<D>::Close() {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Close();
  }
}

#endif
