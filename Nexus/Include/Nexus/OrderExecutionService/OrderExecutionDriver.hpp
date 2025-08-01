#ifndef NEXUS_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_ORDER_EXECUTION_DRIVER_HPP
#include <memory>
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"

namespace Nexus::OrderExecutionService {

  /** Provides a generic interface over an arbitrary OrderExecutionDriver. */
  class OrderExecutionDriver {
    public:

      /**
       * Constructs an OrderExecutionDriver of a specified type using
       * emplacement.
       * @param <T> The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<typename T, typename... Args>
      explicit OrderExecutionDriver(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an OrderExecutionDriver by copying an existing driver.
       * @param driver The driver to copy.
       */
      template<typename Driver>
      explicit OrderExecutionDriver(Driver driver);

      explicit OrderExecutionDriver(OrderExecutionDriver* driver);

      explicit OrderExecutionDriver(
        const std::shared_ptr<OrderExecutionDriver>& driver);

      explicit OrderExecutionDriver(
        const std::unique_ptr<OrderExecutionDriver>& driver);

      std::shared_ptr<const Order> recover(
        const SequencedAccountOrderRecord& record);

      void add(const std::shared_ptr<const Order>& order);

      std::shared_ptr<const Order> submit(const OrderInfo& info);

      void cancel(const OrderExecutionSession& session, OrderId id);

      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);

      void close();

    private:
      struct VirtualOrderExecutionDriver {
        virtual ~VirtualOrderExecutionDriver() = default;
        virtual std::shared_ptr<const Order> recover(
          const SequencedAccountOrderRecord& record) = 0;
        virtual void add(const std::shared_ptr<const Order>& order) = 0;
        virtual std::shared_ptr<const Order> submit(const OrderInfo& info) = 0;
        virtual void cancel(const OrderExecutionSession& session,
          OrderId id) = 0;
        virtual void update(const OrderExecutionSession& session, OrderId id,
          const ExecutionReport& report) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedOrderExecutionDriver final : VirtualOrderExecutionDriver {
        using Driver = D;
        Beam::GetOptionalLocalPtr<Driver> m_driver;

        template<typename... Args>
        WrappedOrderExecutionDriver(Args&&... args);
        std::shared_ptr<const Order> recover(
          const SequencedAccountOrderRecord& record) override;
        void add(const std::shared_ptr<const Order>& order) override;
        std::shared_ptr<const Order> submit(const OrderInfo& info) override;
        void cancel(const OrderExecutionSession& session, OrderId id) override;
        void update(const OrderExecutionSession& session, OrderId id,
          const ExecutionReport& report) override;
        void close() override;
      };
      std::shared_ptr<VirtualOrderExecutionDriver> m_driver;
  };

  /** Checks if a type implements an OrderExecutionDriver. */
  template<typename T>
  concept IsOrderExecutionDriver = std::constructible_from<
    OrderExecutionDriver, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  OrderExecutionDriver::OrderExecutionDriver(
    std::in_place_type_t<T>, Args&&... args)
    : m_driver(std::make_shared<WrappedOrderExecutionDriver<T>>(
        std::forward<Args>(args)...)) {}

  template<typename Driver>
  OrderExecutionDriver::OrderExecutionDriver(Driver driver)
    : OrderExecutionDriver(std::in_place_type<Driver>, std::move(driver)) {}

  inline OrderExecutionDriver::OrderExecutionDriver(
    OrderExecutionDriver* driver)
    : OrderExecutionDriver(*driver) {}

  inline OrderExecutionDriver::OrderExecutionDriver(
    const std::shared_ptr<OrderExecutionDriver>& driver)
    : OrderExecutionDriver(*driver) {}

  inline OrderExecutionDriver::OrderExecutionDriver(
    const std::unique_ptr<OrderExecutionDriver>& driver)
    : OrderExecutionDriver(*driver) {}

  inline std::shared_ptr<const Order> OrderExecutionDriver::recover(
      const SequencedAccountOrderRecord& record) {
    return m_driver->recover(record);
  }

  inline void OrderExecutionDriver::add(
      const std::shared_ptr<const Order>& order) {
    return m_driver->add(order);
  }

  inline std::shared_ptr<const Order> OrderExecutionDriver::submit(
      const OrderInfo& info) {
    return m_driver->submit(info);
  }

  inline void OrderExecutionDriver::cancel(
      const OrderExecutionSession& session, OrderId id) {
    m_driver->cancel(session, id);
  }

  inline void OrderExecutionDriver::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    m_driver->update(session, id, report);
  }

  inline void OrderExecutionDriver::close() {
    m_driver->close();
  }

  template<typename D>
  template<typename... Args>
  OrderExecutionDriver::WrappedOrderExecutionDriver<D>::
    WrappedOrderExecutionDriver(Args&&... args)
    : m_driver(std::forward<Args>(args)...) {}

  template<typename D>
  std::shared_ptr<const Order>
      OrderExecutionDriver::WrappedOrderExecutionDriver<D>::recover(
        const SequencedAccountOrderRecord& record) {
    return m_driver->recover(record);
  }

  template<typename D>
  void OrderExecutionDriver::WrappedOrderExecutionDriver<D>::add(
      const std::shared_ptr<const Order>& order) {
    m_driver->add(order);
  }

  template<typename D>
  std::shared_ptr<const Order>
      OrderExecutionDriver::WrappedOrderExecutionDriver<D>::submit(
        const OrderInfo& info) {
    return m_driver->submit(info);
  }

  template<typename D>
  void OrderExecutionDriver::WrappedOrderExecutionDriver<D>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    m_driver->cancel(session, id);
  }

  template<typename D>
  void OrderExecutionDriver::WrappedOrderExecutionDriver<D>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    m_driver->update(session, id, report);
  }

  template<typename D>
  void OrderExecutionDriver::WrappedOrderExecutionDriver<D>::close() {
    m_driver->close();
  }
}

#endif
