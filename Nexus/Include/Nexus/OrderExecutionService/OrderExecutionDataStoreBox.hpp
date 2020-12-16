#ifndef NEXUS_ORDER_EXECUTION_DATA_STORE_BOX_HPP
#define NEXUS_ORDER_EXECUTION_DATA_STORE_BOX_HPP
#include <memory>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Provides a generic interface over an arbitrary OrderExecutionDataStore. */
  class OrderExecutionDataStoreBox {
    public:

      /**
       * Constructs an OrderExecutionDataStoreBox of a specified type using
       * emplacement.
       * @param <T> The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<typename T, typename... Args>
      explicit OrderExecutionDataStoreBox(std::in_place_type_t<T>,
        Args&&... args);

      /**
       * Constructs an OrderExecutionDataStoreBox by copying an existing data
       * store.
       * @param client The client to copy.
       */
      template<typename DataStore>
      explicit OrderExecutionDataStoreBox(DataStore dataStore);

      explicit OrderExecutionDataStoreBox(
        OrderExecutionDataStoreBox* dataStore);

      explicit OrderExecutionDataStoreBox(
        const std::shared_ptr<OrderExecutionDataStoreBox>& dataStore);

      explicit OrderExecutionDataStoreBox(
        const std::unique_ptr<OrderExecutionDataStoreBox>& dataStore);

      boost::optional<SequencedAccountOrderRecord> LoadOrder(OrderId id);

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query);

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query);

      void Store(const SequencedAccountOrderInfo& orderInfo);

      void Store(const std::vector<SequencedAccountOrderInfo>& orderInfo);

      void Store(const SequencedAccountExecutionReport& executionReport);

      void Store(const std::vector<SequencedAccountExecutionReport>&
        executionReports);

      void Close();

    private:
      struct VirtualOrderExecutionDataStore {
        virtual ~VirtualOrderExecutionDataStore() = default;
        virtual boost::optional<SequencedAccountOrderRecord> LoadOrder(
          OrderId id) = 0;
        virtual std::vector<SequencedOrderRecord> LoadOrderSubmissions(
          const AccountQuery& query) = 0;
        virtual std::vector<SequencedExecutionReport> LoadExecutionReports(
          const AccountQuery& query) = 0;
        virtual void Store(const SequencedAccountOrderInfo& orderInfo) = 0;
        virtual void Store(
          const std::vector<SequencedAccountOrderInfo>& orderInfo) = 0;
        virtual void Store(
          const SequencedAccountExecutionReport& executionReport) = 0;
        virtual void Store(const std::vector<SequencedAccountExecutionReport>&
          executionReports) = 0;
        virtual void Close() = 0;
      };
      template<typename D>
      struct WrappedOrderExecutionDataStore final :
          VirtualOrderExecutionDataStore {
        using OrderExecutionDataStore = D;
        Beam::GetOptionalLocalPtr<OrderExecutionDataStore> m_dataStore;

        template<typename... Args>
        WrappedOrderExecutionDataStore(Args&&... args);
        boost::optional<SequencedAccountOrderRecord> LoadOrder(
          OrderId id) override;
        std::vector<SequencedOrderRecord> LoadOrderSubmissions(
          const AccountQuery& query) override;
        std::vector<SequencedExecutionReport> LoadExecutionReports(
          const AccountQuery& query) override;
        void Store(const SequencedAccountOrderInfo& orderInfo) override;
        void Store(
          const std::vector<SequencedAccountOrderInfo>& orderInfo) override;
        void Store(
          const SequencedAccountExecutionReport& executionReport) override;
        void Store(const std::vector<SequencedAccountExecutionReport>&
          executionReports) override;
        void Close() override;
      };
      std::shared_ptr<VirtualOrderExecutionDataStore> m_dataStore;
  };

  template<typename T, typename... Args>
  OrderExecutionDataStoreBox::OrderExecutionDataStoreBox(
    std::in_place_type_t<T>, Args&&... args)
    : m_dataStore(std::make_shared<WrappedOrderExecutionDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<typename DataStore>
  OrderExecutionDataStoreBox::OrderExecutionDataStoreBox(DataStore dataStore)
    : OrderExecutionDataStoreBox(std::in_place_type<DataStore>,
        std::move(dataStore)) {}

  inline OrderExecutionDataStoreBox::OrderExecutionDataStoreBox(
    OrderExecutionDataStoreBox* dataStore)
    : OrderExecutionDataStoreBox(*dataStore) {}

  inline OrderExecutionDataStoreBox::OrderExecutionDataStoreBox(
    const std::shared_ptr<OrderExecutionDataStoreBox>& dataStore)
    : OrderExecutionDataStoreBox(*dataStore) {}

  inline OrderExecutionDataStoreBox::OrderExecutionDataStoreBox(
    const std::unique_ptr<OrderExecutionDataStoreBox>& dataStore)
    : OrderExecutionDataStoreBox(*dataStore) {}

  inline boost::optional<SequencedAccountOrderRecord>
      OrderExecutionDataStoreBox::LoadOrder(OrderId id) {
    return m_dataStore->LoadOrder(id);
  }

  inline std::vector<SequencedOrderRecord>
      OrderExecutionDataStoreBox::LoadOrderSubmissions(
        const AccountQuery& query) {
    return m_dataStore->LoadOrderSubmissions(query);
  }

  inline std::vector<SequencedExecutionReport>
      OrderExecutionDataStoreBox::LoadExecutionReports(
        const AccountQuery& query) {
    return m_dataStore->LoadExecutionReports(query);
  }

  inline void OrderExecutionDataStoreBox::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_dataStore->Store(orderInfo);
  }

  inline void OrderExecutionDataStoreBox::Store(
      const std::vector<SequencedAccountOrderInfo>& orderInfo) {
    m_dataStore->Store(orderInfo);
  }

  inline void OrderExecutionDataStoreBox::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_dataStore->Store(executionReport);
  }

  inline void OrderExecutionDataStoreBox::Store(
      const std::vector<SequencedAccountExecutionReport>& executionReports) {
    m_dataStore->Store(executionReports);
  }

  inline void OrderExecutionDataStoreBox::Close() {
    m_dataStore->Close();
  }

  template<typename D>
  template<typename... Args>
  OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<D>::
    WrappedOrderExecutionDataStore(Args&&... args)
    : m_dataStore(std::forward<Args>(args)...) {}

  template<typename D>
  boost::optional<SequencedAccountOrderRecord>
      OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<D>::LoadOrder(
        OrderId id) {
    return m_dataStore->LoadOrder(id);
  }

  template<typename D>
  std::vector<SequencedOrderRecord>
      OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<
        D>::LoadOrderSubmissions(const AccountQuery& query) {
    return m_dataStore->LoadOrderSubmissions(query);
  }

  template<typename D>
  std::vector<SequencedExecutionReport>
      OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<
        D>::LoadExecutionReports(const AccountQuery& query) {
    return m_dataStore->LoadExecutionReports(query);
  }

  template<typename D>
  void OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<D>::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_dataStore->Store(orderInfo);
  }

  template<typename D>
  void OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<D>::Store(
      const std::vector<SequencedAccountOrderInfo>& orderInfo) {
    m_dataStore->Store(orderInfo);
  }

  template<typename D>
  void OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<D>::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_dataStore->Store(executionReport);
  }

  template<typename D>
  void OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<D>::Store(
      const std::vector<SequencedAccountExecutionReport>& executionReports) {
    m_dataStore->Store(executionReports);
  }

  template<typename D>
  void OrderExecutionDataStoreBox::WrappedOrderExecutionDataStore<D>::Close() {
    m_dataStore->Close();
  }
}

#endif
