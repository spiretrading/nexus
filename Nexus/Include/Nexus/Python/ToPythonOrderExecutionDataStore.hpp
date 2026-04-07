#ifndef NEXUS_PYTHON_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_DATA_STORE_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"

namespace Nexus {

  /**
   * Wraps an OrderExecutionDataStore for use with Python.
   * @param <D> The type of OrderExecutionDataStore to wrap.
   */
  template<IsOrderExecutionDataStore D>
  class ToPythonOrderExecutionDataStore {
    public:

      /** The type of data store to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonOrderExecutionDataStore in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonOrderExecutionDataStore(Args&&... args);

      ~ToPythonOrderExecutionDataStore();

      /** Returns a reference to the underlying data store. */
      DataStore& get();

      /** Returns a reference to the underlying data store. */
      const DataStore& get() const;

      boost::optional<SequencedAccountOrderRecord>
        load_order_record(OrderId id);
      std::vector<SequencedOrderRecord>
        load_order_records(const AccountQuery& query);
      void store(const SequencedAccountOrderInfo& info);
      void store(const std::vector<SequencedAccountOrderInfo>& info);
      std::vector<SequencedExecutionReport>
        load_execution_reports(const AccountQuery& query);
      void store(const SequencedAccountExecutionReport& report);
      void store(const std::vector<SequencedAccountExecutionReport>& reports);
      void close();

    private:
      boost::optional<DataStore> m_data_store;

      ToPythonOrderExecutionDataStore(
        const ToPythonOrderExecutionDataStore&) = delete;
      ToPythonOrderExecutionDataStore& operator =(
        const ToPythonOrderExecutionDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonOrderExecutionDataStore(DataStore&&) ->
    ToPythonOrderExecutionDataStore<std::remove_cvref_t<DataStore>>;

  template<IsOrderExecutionDataStore D>
  template<typename... Args>
  ToPythonOrderExecutionDataStore<D>::ToPythonOrderExecutionDataStore(
    Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsOrderExecutionDataStore D>
  ToPythonOrderExecutionDataStore<D>::~ToPythonOrderExecutionDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<IsOrderExecutionDataStore D>
  typename ToPythonOrderExecutionDataStore<D>::DataStore&
      ToPythonOrderExecutionDataStore<D>::get() {
    return *m_data_store;
  }

  template<IsOrderExecutionDataStore D>
  const typename ToPythonOrderExecutionDataStore<D>::DataStore&
      ToPythonOrderExecutionDataStore<D>::get() const {
    return *m_data_store;
  }

  template<IsOrderExecutionDataStore D>
  boost::optional<SequencedAccountOrderRecord>
      ToPythonOrderExecutionDataStore<D>::load_order_record(OrderId id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_order_record(id);
  }

  template<IsOrderExecutionDataStore D>
  std::vector<SequencedOrderRecord>
      ToPythonOrderExecutionDataStore<D>::load_order_records(
        const AccountQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_order_records(query);
  }

  template<IsOrderExecutionDataStore D>
  void ToPythonOrderExecutionDataStore<D>::store(
      const SequencedAccountOrderInfo& info) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(info);
  }

  template<IsOrderExecutionDataStore D>
  void ToPythonOrderExecutionDataStore<D>::store(
      const std::vector<SequencedAccountOrderInfo>& info) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(info);
  }

  template<IsOrderExecutionDataStore D>
  std::vector<SequencedExecutionReport>
      ToPythonOrderExecutionDataStore<D>::load_execution_reports(
        const AccountQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_execution_reports(query);
  }

  template<IsOrderExecutionDataStore D>
  void ToPythonOrderExecutionDataStore<D>::store(
      const SequencedAccountExecutionReport& report) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(report);
  }

  template<IsOrderExecutionDataStore D>
  void ToPythonOrderExecutionDataStore<D>::store(
      const std::vector<SequencedAccountExecutionReport>& reports) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(reports);
  }

  template<IsOrderExecutionDataStore D>
  void ToPythonOrderExecutionDataStore<D>::close() {
    auto release = Beam::Python::GilRelease();
    m_data_store->close();
  }
}

#endif
