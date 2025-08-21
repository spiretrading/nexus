#ifndef NEXUS_PYTHON_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"

namespace Nexus::OrderExecutionService {

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
       * Constructs a ToPythonOrderExecutionDataStore.
       * @param args The arguments to forward to the DataStore's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonOrderExecutionDataStore,
          Args...>>
      ToPythonOrderExecutionDataStore(Args&&... args);

      ~ToPythonOrderExecutionDataStore();

      /** Returns the wrapped data store. */
      const DataStore& get_data_store() const;

      /** Returns the wrapped data store. */
      DataStore& get_data_store();
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
    ToPythonOrderExecutionDataStore<std::remove_reference_t<DataStore>>;

  template<IsOrderExecutionDataStore D>
  template<typename... Args, typename>
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
  const typename ToPythonOrderExecutionDataStore<D>::DataStore&
      ToPythonOrderExecutionDataStore<D>::get_data_store() const {
    return *m_data_store;
  }

  template<IsOrderExecutionDataStore D>
  typename ToPythonOrderExecutionDataStore<D>::DataStore&
      ToPythonOrderExecutionDataStore<D>::get_data_store() {
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
