#ifndef NEXUS_VIRTUALORDEREXECUTIONDATASTORE_HPP
#define NEXUS_VIRTUALORDEREXECUTIONDATASTORE_HPP
#include <memory>
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class VirtualOrderExecutionDataStore
      \brief Provides a pure virtual interface to an OrderExecutionDataStore.
   */
  class VirtualOrderExecutionDataStore : private boost::noncopyable {
    public:
      virtual ~VirtualOrderExecutionDataStore() = default;

      virtual std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query) = 0;

      virtual std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query) = 0;

      virtual void Store(const SequencedAccountOrderInfo& orderInfo) = 0;

      virtual void Store(
        const SequencedAccountExecutionReport& executionReport) = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualOrderExecutionDataStore.
      VirtualOrderExecutionDataStore() = default;
  };

  /*! \class WrapperOrderExecutionDataStore
      \brief Wraps an OrderExecutionDataStore providing it with a virtual
             interface.
      \tparam DataStoreType The type of OrderExecutionDataStore to wrap.
   */
  template<typename DataStoreType>
  class WrapperOrderExecutionDataStore : public VirtualOrderExecutionDataStore {
    public:

      //! The OrderExecutionDataStore to wrap.
      using DataStore = Beam::GetTryDereferenceType<DataStoreType>;

      //! Constructs a WrapperOrderExecutionDataStore.
      /*!
        \param dataStore The OrderExecutionDataStore to wrap.
      */
      template<typename OrderExecutionDataStoreForward>
      WrapperOrderExecutionDataStore(
        OrderExecutionDataStoreForward&& dataStore);

      virtual ~WrapperOrderExecutionDataStore() override final;

      virtual std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query) override final;

      virtual std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query) override final;

      virtual void Store(const SequencedAccountOrderInfo& orderInfo)
        override final;

      virtual void Store(
        const SequencedAccountExecutionReport& executionReport) override final;

      virtual void Close() override final;

    private:
      Beam::GetOptionalLocalPtr<DataStoreType> m_dataStore;
  };

  //! Wraps an OrderExecutionDataStore into a VirtualOrderExecutionDataStore.
  /*!
    \param dataStore The data store to wrap.
  */
  template<typename OrderExecutionDataStore>
  std::unique_ptr<VirtualOrderExecutionDataStore>
      MakeVirtualOrderExecutionDataStore(OrderExecutionDataStore&& dataStore) {
    return std::make_unique<WrapperOrderExecutionDataStore<
      OrderExecutionDataStore>>(
      std::forward<OrderExecutionDataStore>(dataStore));
  }

  template<typename DataStoreType>
  template<typename OrderExecutionDataStoreForward>
  WrapperOrderExecutionDataStore<DataStoreType>::WrapperOrderExecutionDataStore(
      OrderExecutionDataStoreForward&& dataStore)
      : m_dataStore{std::forward<OrderExecutionDataStoreForward>(dataStore)} {}

  template<typename DataStoreType>
  WrapperOrderExecutionDataStore<DataStoreType>::
      ~WrapperOrderExecutionDataStore() {
    Close();
  }

  template<typename DataStoreType>
  std::vector<SequencedOrderRecord>
      WrapperOrderExecutionDataStore<DataStoreType>::LoadOrderSubmissions(
      const AccountQuery& query) {
    return m_dataStore->LoadOrderSubmissions(query);
  }

  template<typename DataStoreType>
  std::vector<SequencedExecutionReport>
      WrapperOrderExecutionDataStore<DataStoreType>::LoadExecutionReports(
      const AccountQuery& query) {
    return m_dataStore->LoadExecutionReports(query);
  }

  template<typename DataStoreType>
  void WrapperOrderExecutionDataStore<DataStoreType>::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    return m_dataStore->Store(orderInfo);
  }

  template<typename DataStoreType>
  void WrapperOrderExecutionDataStore<DataStoreType>::Store(
      const SequencedAccountExecutionReport& executionReport) {
    return m_dataStore->Store(executionReport);
  }

  template<typename DataStoreType>
  void WrapperOrderExecutionDataStore<DataStoreType>::Close() {
    m_dataStore->Close();
  }
}
}

#endif
