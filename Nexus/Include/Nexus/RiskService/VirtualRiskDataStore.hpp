#ifndef NEXUS_VIRTUAL_RISK_DATA_STORE_HPP
#define NEXUS_VIRTUAL_RISK_DATA_STORE_HPP
#include <utility>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus::RiskService {

  /** Provides an abstract interface to a RiskDataStore. */
  class VirtualRiskDataStore : private boost::noncopyable {
    public:
      virtual ~VirtualRiskDataStore() = default;

      virtual InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot) = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualRiskDataStore. */
      VirtualRiskDataStore() = default;
  };

  /**
   * Wraps a RiskDataStore providing it with a virtual interface.
   * @param <D> The type of RiskDataStore to wrap.
   */
  template<typename D>
  class WrapperRiskDataStore : public VirtualRiskDataStore {
    public:

      /** The RiskDataStore to wrap. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a WrapperRiskDataStore.
       * @param dataStore The RiskDataStore to wrap.
       */
      template<typename DF>
      explicit WrapperRiskDataStore(DF&& dataStore);

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot) override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<D> m_dataStore;
  };

  /**
   * Wraps a RiskDataStore into a VirtualRiskDataStore.
   * @param dataStore The data store to wrap.
   */
  template<typename RiskDataStore>
  std::unique_ptr<VirtualRiskDataStore> MakeVirtualRiskDataStore(
      RiskDataStore&& dataStore) {
    return std::make_unique<WrapperRiskDataStore<RiskDataStore>>(
      std::forward<RiskDataStore>(dataStore));
  }

  template<typename D>
  template<typename DF>
  WrapperRiskDataStore<D>::WrapperRiskDataStore(DF&& dataStore)
    : m_dataStore(std::forward<DF>(dataStore)) {}

  template<typename D>
  InventorySnapshot WrapperRiskDataStore<D>::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_dataStore->LoadInventorySnapshot(account);
  }

  template<typename D>
  void WrapperRiskDataStore<D>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    return m_dataStore->Store(account, snapshot);
  }

  template<typename D>
  void WrapperRiskDataStore<D>::Close() {
    m_dataStore->Close();
  }
}

#endif
