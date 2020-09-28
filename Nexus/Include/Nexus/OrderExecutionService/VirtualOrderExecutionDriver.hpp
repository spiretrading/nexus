#ifndef NEXUS_VIRTUALORDEREXECUTIONDRIVER_HPP
#define NEXUS_VIRTUALORDEREXECUTIONDRIVER_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class VirtualOrderExecutionDriver
      \brief Provides a pure virtual interface to an OrderExecutionDriver.
   */
  class VirtualOrderExecutionDriver : private boost::noncopyable {
    public:
      virtual ~VirtualOrderExecutionDriver() = default;

      virtual const Order& Recover(
        const SequencedAccountOrderRecord& order) = 0;

      virtual const Order& Submit(const OrderInfo& orderInfo) = 0;

      virtual void Cancel(const OrderExecutionSession& session,
        OrderId orderId) = 0;

      virtual void Update(const OrderExecutionSession& session,
        OrderId orderId, const ExecutionReport& executionReport) = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualOrderExecutionDriver.
      VirtualOrderExecutionDriver() = default;
  };

  /*! \class WrapperOrderExecutionDriver
      \brief Wraps an OrderExecutionDriver providing it with a virtual
             interface.
      \tparam DriverType The type of OrderExecutionDriver to wrap.
   */
  template<typename DriverType>
  class WrapperOrderExecutionDriver : public VirtualOrderExecutionDriver {
    public:

      //! The OrderExecutionDriver to wrap.
      using Driver = Beam::GetTryDereferenceType<DriverType>;

      //! Constructs a WrapperOrderExecutionDriver.
      /*!
        \param driver The OrderExecutionDriver to wrap.
      */
      template<typename OrderExecutionDriverForward>
      WrapperOrderExecutionDriver(OrderExecutionDriverForward&& driver);

      virtual ~WrapperOrderExecutionDriver() override = default;

      //! Returns the driver being wrapped.
      const Driver& GetDriver() const;

      //! Returns the driver being wrapped.
      Driver& GetDriver();

      virtual const Order& Recover(
        const SequencedAccountOrderRecord& order) override;

      virtual const Order& Submit(const OrderInfo& orderInfo) override;

      virtual void Cancel(const OrderExecutionSession& session,
        OrderId orderId) override;

      virtual void Update(const OrderExecutionSession& session,
        OrderId orderId, const ExecutionReport& executionReport) override;

      virtual void Close() override;

    private:
      Beam::GetOptionalLocalPtr<DriverType> m_driver;
  };

  //! Wraps an OrderExecutionDriver into a VirtualOrderExecutionDriver.
  /*!
    \param driver The driver to wrap.
  */
  template<typename OrderExecutionDriver>
  std::unique_ptr<VirtualOrderExecutionDriver> MakeVirtualOrderExecutionDriver(
      OrderExecutionDriver&& driver) {
    return std::make_unique<WrapperOrderExecutionDriver<OrderExecutionDriver>>(
      std::forward<OrderExecutionDriver>(driver));
  }

  //! Wraps an OrderExecutionDriver into a VirtualOrderExecutionDriver.
  /*!
    \param initializer Initializes the driver being wrapped.
  */
  template<typename OrderExecutionDriver, typename... Args>
  std::unique_ptr<VirtualOrderExecutionDriver> MakeVirtualOrderExecutionDriver(
      Beam::Initializer<Args...>&& initializer) {
    return std::make_unique<WrapperOrderExecutionDriver<OrderExecutionDriver>>(
      std::move(initializer));
  }

  template<typename DriverType>
  template<typename OrderExecutionDriverForward>
  WrapperOrderExecutionDriver<DriverType>::WrapperOrderExecutionDriver(
      OrderExecutionDriverForward&& driver)
      : m_driver{std::forward<OrderExecutionDriverForward>(driver)} {}

  template<typename DriverType>
  const typename WrapperOrderExecutionDriver<DriverType>::Driver&
      WrapperOrderExecutionDriver<DriverType>::GetDriver() const {
    return *m_driver;
  }

  template<typename DriverType>
  typename WrapperOrderExecutionDriver<DriverType>::Driver&
      WrapperOrderExecutionDriver<DriverType>::GetDriver() {
    return *m_driver;
  }

  template<typename DriverType>
  const Order& WrapperOrderExecutionDriver<DriverType>::Recover(
      const SequencedAccountOrderRecord& order) {
    return m_driver->Recover(order);
  }

  template<typename DriverType>
  const Order& WrapperOrderExecutionDriver<DriverType>::Submit(
      const OrderInfo& orderInfo) {
    return m_driver->Submit(orderInfo);
  }

  template<typename DriverType>
  void WrapperOrderExecutionDriver<DriverType>::Cancel(
      const OrderExecutionSession& session, OrderId orderId) {
    m_driver->Cancel(session, orderId);
  }

  template<typename DriverType>
  void WrapperOrderExecutionDriver<DriverType>::Update(
      const OrderExecutionSession& session, OrderId orderId,
      const ExecutionReport& executionReport) {
    m_driver->Update(session, orderId, executionReport);
  }

  template<typename DriverType>
  void WrapperOrderExecutionDriver<DriverType>::Close() {
    m_driver->Close();
  }
}
}

#endif
