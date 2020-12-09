#ifndef NEXUS_PYTHON_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreBox.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Wraps an OrderExecutionDataStore for use with Python.
   * @param <D> The type of OrderExecutionDataStore to wrap.
   */
  template<typename D>
  class ToPythonOrderExecutionDataStore {
    public:

      /** The type of data store to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonOrderExecutionDataStore.
       * @param args The arguments to forward to the DataStore's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonOrderExecutionDataStore,
          Args...>>
      ToPythonOrderExecutionDataStore(Args&&... args);

      ~ToPythonOrderExecutionDataStore();

      /** Returns the wrapped data store. */
      const DataStore& GetDataStore() const;

      /** Returns the wrapped data store. */
      DataStore& GetDataStore();

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
      boost::optional<DataStore> m_dataStore;

      ToPythonOrderExecutionDataStore(
        const ToPythonOrderExecutionDataStore&) = delete;
      ToPythonOrderExecutionDataStore& operator =(
        const ToPythonOrderExecutionDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonOrderExecutionDataStore(DataStore&&) ->
    ToPythonOrderExecutionDataStore<std::decay_t<DataStore>>;

  template<typename D>
  template<typename... Args, typename>
  ToPythonOrderExecutionDataStore<D>::ToPythonOrderExecutionDataStore(
    Args&&... args)
    : m_dataStore((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename D>
  ToPythonOrderExecutionDataStore<D>::~ToPythonOrderExecutionDataStore() {
    auto release = Beam::Python::GilRelease();
    m_dataStore.reset();
  }

  template<typename D>
  const typename ToPythonOrderExecutionDataStore<D>::DataStore&
      ToPythonOrderExecutionDataStore<D>::GetDataStore() const {
    return *m_dataStore;
  }

  template<typename D>
  typename ToPythonOrderExecutionDataStore<D>::DataStore&
      ToPythonOrderExecutionDataStore<D>::GetDataStore() {
    return *m_dataStore;
  }

  template<typename D>
  boost::optional<SequencedAccountOrderRecord>
      ToPythonOrderExecutionDataStore<D>::LoadOrder(OrderId id) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadOrder(id);
  }

  template<typename D>
  std::vector<SequencedOrderRecord>
      ToPythonOrderExecutionDataStore<D>::LoadOrderSubmissions(
        const AccountQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadOrderSubmissions(query);
  }

  template<typename D>
  std::vector<SequencedExecutionReport>
      ToPythonOrderExecutionDataStore<D>::LoadExecutionReports(
        const AccountQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadExecutionReports(query);
  }

  template<typename D>
  void ToPythonOrderExecutionDataStore<D>::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(orderInfo);
  }

  template<typename D>
  void ToPythonOrderExecutionDataStore<D>::Store(
      const std::vector<SequencedAccountOrderInfo>& orderInfo) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(orderInfo);
  }

  template<typename D>
  void ToPythonOrderExecutionDataStore<D>::Store(
      const SequencedAccountExecutionReport& executionReport) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(executionReport);
  }

  template<typename D>
  void ToPythonOrderExecutionDataStore<D>::Store(
      const std::vector<SequencedAccountExecutionReport>& executionReports) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(executionReports);
  }

  template<typename D>
  void ToPythonOrderExecutionDataStore<D>::Close() {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Close();
  }
}

#endif
