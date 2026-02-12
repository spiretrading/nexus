#ifndef NEXUS_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_ORDER_EXECUTION_DRIVER_HPP
#include <concepts>
#include <memory>
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"

namespace Nexus {

  /** Checks if a type implements an OrderExecutionDriver. */
  template<typename T>
  concept IsOrderExecutionDriver = requires(T& driver) {
    { driver.recover(std::declval<const SequencedAccountOrderRecord&>()) } ->
        std::same_as<std::shared_ptr<Order>>;
    driver.add(std::declval<const std::shared_ptr<Order>&>());
    { driver.submit(std::declval<const OrderInfo&>()) } ->
        std::same_as<std::shared_ptr<Order>>;
    driver.cancel(std::declval<const OrderExecutionSession&>(),
      std::declval<OrderId>());
    driver.update(std::declval<const OrderExecutionSession&>(),
      std::declval<OrderId>(), std::declval<const ExecutionReport&>());
    driver.close();
  };

  /** Provides a generic interface over an arbitrary OrderExecutionDriver. */
  class OrderExecutionDriver {
    public:

      /**
       * Constructs an OrderExecutionDriver of a specified type using
       * emplacement.
       * @tparam T The type of driver to emplace.
       * @param args The arguments to pass to the emplaced driver.
       */
      template<IsOrderExecutionDriver T, typename... Args>
      explicit OrderExecutionDriver(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an OrderExecutionDriver by referencing an existing driver.
       * @param driver The driver to reference.
       */
      template<Beam::DisableCopy<OrderExecutionDriver> T> requires
        IsOrderExecutionDriver<Beam::dereference_t<T>>
      OrderExecutionDriver(T&& driver);

      OrderExecutionDriver(const OrderExecutionDriver&) = default;
      OrderExecutionDriver(OrderExecutionDriver&&) = default;

      /** Returns a reference to the concrete implementation. */
      template<typename T>
      const T& as() const;

      /** Returns a reference to the concrete implementation. */
      template<typename T>
      T& as();

      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      struct VirtualOrderExecutionDriver {
        virtual ~VirtualOrderExecutionDriver() = default;

        virtual std::shared_ptr<Order> recover(
          const SequencedAccountOrderRecord&) = 0;
        virtual void add(const std::shared_ptr<Order>&) = 0;
        virtual std::shared_ptr<Order> submit(const OrderInfo&) = 0;
        virtual void cancel(const OrderExecutionSession&, OrderId) = 0;
        virtual void update(
          const OrderExecutionSession&, OrderId, const ExecutionReport&) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedOrderExecutionDriver final : VirtualOrderExecutionDriver {
        using Driver = D;
        Beam::local_ptr_t<Driver> m_driver;

        template<typename... Args>
        WrappedOrderExecutionDriver(Args&&... args);

        std::shared_ptr<Order> recover(
          const SequencedAccountOrderRecord& record) override;
        void add(const std::shared_ptr<Order>& order) override;
        std::shared_ptr<Order> submit(const OrderInfo& info) override;
        void cancel(const OrderExecutionSession& session, OrderId id) override;
        void update(const OrderExecutionSession& session, OrderId id,
          const ExecutionReport& report) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualOrderExecutionDriver> m_driver;
  };

  template<IsOrderExecutionDriver T, typename... Args>
  OrderExecutionDriver::OrderExecutionDriver(
    std::in_place_type_t<T>, Args&&... args)
    : m_driver(Beam::make_virtual_ptr<WrappedOrderExecutionDriver<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<OrderExecutionDriver> T> requires
    IsOrderExecutionDriver<Beam::dereference_t<T>>
  OrderExecutionDriver::OrderExecutionDriver(T&& driver)
    : m_driver(Beam::make_virtual_ptr<WrappedOrderExecutionDriver<
        std::remove_cvref_t<T>>>(std::forward<T>(driver))) {}

  template<typename T>
  const T& OrderExecutionDriver::as() const {
    return const_cast<OrderExecutionDriver&>(*this).as<T>();
  }

  template<typename T>
  T& OrderExecutionDriver::as() {
    return *static_cast<WrappedOrderExecutionDriver<T>&>(*m_driver).m_driver;
  }

  inline std::shared_ptr<Order> OrderExecutionDriver::recover(
      const SequencedAccountOrderRecord& record) {
    return m_driver->recover(record);
  }

  inline void OrderExecutionDriver::add(const std::shared_ptr<Order>& order) {
    return m_driver->add(order);
  }

  inline std::shared_ptr<Order> OrderExecutionDriver::submit(
      const OrderInfo& info) {
    return m_driver->submit(info);
  }

  inline void OrderExecutionDriver::cancel(
      const OrderExecutionSession& session, OrderId id) {
    m_driver->cancel(session, id);
  }

  inline void OrderExecutionDriver::update(const OrderExecutionSession& session,
      OrderId id, const ExecutionReport& report) {
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
  std::shared_ptr<Order>
      OrderExecutionDriver::WrappedOrderExecutionDriver<D>::recover(
        const SequencedAccountOrderRecord& record) {
    return m_driver->recover(record);
  }

  template<typename D>
  void OrderExecutionDriver::WrappedOrderExecutionDriver<D>::add(
      const std::shared_ptr<Order>& order) {
    m_driver->add(order);
  }

  template<typename D>
  std::shared_ptr<Order>
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
