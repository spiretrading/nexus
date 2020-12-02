#ifndef NEXUS_VIRTUAL_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_VIRTUAL_ORDER_EXECUTION_DRIVER_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Provides a pure virtual interface to an OrderExecutionDriver. */
  class VirtualOrderExecutionDriver {
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

      /** Constructs a VirtualOrderExecutionDriver. */
      VirtualOrderExecutionDriver() = default;

    private:
      VirtualOrderExecutionDriver(const VirtualOrderExecutionDriver&) = delete;
      VirtualOrderExecutionDriver& operator =(
        const VirtualOrderExecutionDriver&) = delete;
  };

  /**
   * Wraps an OrderExecutionDriver providing it with a virtual interface.
   * @param <D> The type of OrderExecutionDriver to wrap.
   */
  template<typename D>
  class WrapperOrderExecutionDriver : public VirtualOrderExecutionDriver {
    public:

      /** The OrderExecutionDriver to wrap. */
      using Driver = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a WrapperOrderExecutionDriver.
       * @param driver The OrderExecutionDriver to wrap.
       */
      template<typename DF>
      WrapperOrderExecutionDriver(DF&& driver);

      /** Returns the driver being wrapped. */
      const Driver& GetDriver() const;

      /** Returns the driver being wrapped. */
      Driver& GetDriver();

      const Order& Recover(const SequencedAccountOrderRecord& order) override;

      const Order& Submit(const OrderInfo& orderInfo) override;

      void Cancel(const OrderExecutionSession& session,
        OrderId orderId) override;

      void Update(const OrderExecutionSession& session, OrderId orderId,
        const ExecutionReport& executionReport) override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<D> m_driver;
  };

  /**
   * Wraps an OrderExecutionDriver into a VirtualOrderExecutionDriver.
   * @param driver The driver to wrap.
   */
  template<typename OrderExecutionDriver>
  std::unique_ptr<VirtualOrderExecutionDriver> MakeVirtualOrderExecutionDriver(
      OrderExecutionDriver&& driver) {
    return std::make_unique<WrapperOrderExecutionDriver<OrderExecutionDriver>>(
      std::forward<OrderExecutionDriver>(driver));
  }

  /**
   * Wraps an OrderExecutionDriver into a VirtualOrderExecutionDriver.
   * @param initializer Initializes the driver being wrapped.
   */
  template<typename OrderExecutionDriver, typename... Args>
  std::unique_ptr<VirtualOrderExecutionDriver> MakeVirtualOrderExecutionDriver(
      Beam::Initializer<Args...>&& initializer) {
    return std::make_unique<WrapperOrderExecutionDriver<OrderExecutionDriver>>(
      std::move(initializer));
  }

  template<typename D>
  template<typename DF>
  WrapperOrderExecutionDriver<D>::WrapperOrderExecutionDriver(DF&& driver)
    : m_driver(std::forward<DF>(driver)) {}

  template<typename D>
  const typename WrapperOrderExecutionDriver<D>::Driver&
      WrapperOrderExecutionDriver<D>::GetDriver() const {
    return *m_driver;
  }

  template<typename D>
  typename WrapperOrderExecutionDriver<D>::Driver&
      WrapperOrderExecutionDriver<D>::GetDriver() {
    return *m_driver;
  }

  template<typename D>
  const Order& WrapperOrderExecutionDriver<D>::Recover(
      const SequencedAccountOrderRecord& order) {
    return m_driver->Recover(order);
  }

  template<typename D>
  const Order& WrapperOrderExecutionDriver<D>::Submit(
      const OrderInfo& orderInfo) {
    return m_driver->Submit(orderInfo);
  }

  template<typename D>
  void WrapperOrderExecutionDriver<D>::Cancel(
      const OrderExecutionSession& session, OrderId orderId) {
    m_driver->Cancel(session, orderId);
  }

  template<typename D>
  void WrapperOrderExecutionDriver<D>::Update(
      const OrderExecutionSession& session, OrderId orderId,
      const ExecutionReport& executionReport) {
    m_driver->Update(session, orderId, executionReport);
  }

  template<typename D>
  void WrapperOrderExecutionDriver<D>::Close() {
    m_driver->Close();
  }
}

#endif
