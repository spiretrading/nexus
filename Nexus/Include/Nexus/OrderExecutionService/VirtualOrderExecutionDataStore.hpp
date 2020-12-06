#ifndef NEXUS_VIRTUAL_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_VIRTUAL_ORDER_EXECUTION_DATA_STORE_HPP
#include <memory>
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Provides a pure virtual interface to an OrderExecutionDataStore. */
  class VirtualOrderExecutionDataStore {
    public:
      virtual ~VirtualOrderExecutionDataStore() = default;

      virtual boost::optional<SequencedOrderRecord> LoadOrder(OrderId id) = 0;

      virtual std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query) = 0;

      virtual std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query) = 0;

      virtual void Store(const SequencedAccountOrderInfo& orderInfo) = 0;

      virtual void Store(
        const SequencedAccountExecutionReport& executionReport) = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualOrderExecutionDataStore. */
      VirtualOrderExecutionDataStore() = default;

    private:
      VirtualOrderExecutionDataStore(
        const VirtualOrderExecutionDataStore&) = delete;
      VirtualOrderExecutionDataStore& operator =(
        const VirtualOrderExecutionDataStore&) = delete;
  };

  /**
   * Wraps an OrderExecutionDataStore providing it with a virtual interface.
   * @param <D> The type of OrderExecutionDataStore to wrap.
   */
  template<typename D>
  class WrapperOrderExecutionDataStore final :
      public VirtualOrderExecutionDataStore {
    public:

      /** The OrderExecutionDataStore to wrap. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a WrapperOrderExecutionDataStore.
       * @param dataStore The OrderExecutionDataStore to wrap.
       */
      template<typename DF>
      WrapperOrderExecutionDataStore(DF&& dataStore);

      ~WrapperOrderExecutionDataStore() override;

      boost::optional<SequencedOrderRecord> LoadOrder(OrderId id) override;

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query) override;

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query) override;

      void Store(const SequencedAccountOrderInfo& orderInfo) override;

      void Store(
        const SequencedAccountExecutionReport& executionReport) override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<D> m_dataStore;
  };

  /**
   * Wraps an OrderExecutionDataStore into a VirtualOrderExecutionDataStore.
   * @param dataStore The data store to wrap.
   */
  template<typename OrderExecutionDataStore>
  std::unique_ptr<VirtualOrderExecutionDataStore>
      MakeVirtualOrderExecutionDataStore(OrderExecutionDataStore&& dataStore) {
    return std::make_unique<WrapperOrderExecutionDataStore<
      OrderExecutionDataStore>>(
      std::forward<OrderExecutionDataStore>(dataStore));
  }

  template<typename D>
  template<typename DF>
  WrapperOrderExecutionDataStore<D>::WrapperOrderExecutionDataStore(
    DF&& dataStore)
    : m_dataStore(std::forward<DF>(dataStore)) {}

  template<typename D>
  WrapperOrderExecutionDataStore<D>::~WrapperOrderExecutionDataStore() {
    Close();
  }

  template<typename D>
  boost::optional<SequencedOrderRecord>
      WrapperOrderExecutionDataStore<D>::LoadOrder(OrderId id) {
    return m_dataStore->LoadOrder(id);
  }

  template<typename D>
  std::vector<SequencedOrderRecord>
      WrapperOrderExecutionDataStore<D>::LoadOrderSubmissions(
      const AccountQuery& query) {
    return m_dataStore->LoadOrderSubmissions(query);
  }

  template<typename D>
  std::vector<SequencedExecutionReport>
      WrapperOrderExecutionDataStore<D>::LoadExecutionReports(
      const AccountQuery& query) {
    return m_dataStore->LoadExecutionReports(query);
  }

  template<typename D>
  void WrapperOrderExecutionDataStore<D>::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    return m_dataStore->Store(orderInfo);
  }

  template<typename D>
  void WrapperOrderExecutionDataStore<D>::Store(
      const SequencedAccountExecutionReport& executionReport) {
    return m_dataStore->Store(executionReport);
  }

  template<typename D>
  void WrapperOrderExecutionDataStore<D>::Close() {
    m_dataStore->Close();
  }
}

#endif
