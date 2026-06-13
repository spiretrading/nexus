#ifndef NEXUS_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_ORDER_EXECUTION_DRIVER_HPP
#include <concepts>
#include <memory>
#include <utility>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include "Nexus/Accounting/InventorySnapshot.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"

namespace Nexus {

  /** Checks if a type implements an OrderExecutionDriver. */
  template<typename T>
  concept IsOrderExecutionDriver = requires(T& driver) {
    { driver.restore(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const InventorySnapshot&>(),
        std::declval<const std::vector<SequencedOrderRecord>&>()) } ->
          std::same_as<std::vector<std::shared_ptr<Order>>>;
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

      /**
       * Restores an account's state from a snapshot.
       * @param account The account to restore.
       * @param snapshot The snapshot used to restore the account.
       * @param records The OrderRecords to restore.
       * @return The Orders reconstructed from the snapshot.
       */
      std::vector<std::shared_ptr<Order>> restore(
        const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot,
        const std::vector<SequencedOrderRecord>& records);

      /**
       * Adds an Order to be tracked by this driver.
       * @param order The Order to add.
       */
      void add(const std::shared_ptr<Order>& order);

      /**
       * Submits an Order.
       * @param info The OrderInfo containing the details of the submission.
       * @return The Order that was submitted.
       */
      std::shared_ptr<Order> submit(const OrderInfo& info);

      /**
       * Cancels an Order.
       * @param session The session requesting the cancel.
       * @param id The id of the Order to cancel.
       */
      void cancel(const OrderExecutionSession& session, OrderId id);

      /**
       * Updates an Order with an ExecutionReport.
       * @param session The session requesting the update.
       * @param id The id of the Order to update.
       * @param report The ExecutionReport containing the update.
       */
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);

      /** Closes the driver. */
      void close();

    private:
      struct VirtualOrderExecutionDriver {
        virtual ~VirtualOrderExecutionDriver() = default;

        virtual std::vector<std::shared_ptr<Order>> restore(
          const Beam::DirectoryEntry& account,
          const InventorySnapshot& snapshot,
          const std::vector<SequencedOrderRecord>& records) = 0;
        virtual void add(const std::shared_ptr<Order>& order) = 0;
        virtual std::shared_ptr<Order> submit(const OrderInfo& info) = 0;
        virtual void cancel(
          const OrderExecutionSession& session, OrderId id) = 0;
        virtual void update(const OrderExecutionSession& session, OrderId id,
          const ExecutionReport& report) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedOrderExecutionDriver final : VirtualOrderExecutionDriver {
        using Driver = D;
        Beam::local_ptr_t<Driver> m_driver;

        template<typename... Args>
        WrappedOrderExecutionDriver(Args&&... args);

        std::vector<std::shared_ptr<Order>> restore(
          const Beam::DirectoryEntry& account,
          const InventorySnapshot& snapshot,
          const std::vector<SequencedOrderRecord>& records) override;
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

  inline std::vector<std::shared_ptr<Order>> OrderExecutionDriver::restore(
      const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot,
      const std::vector<SequencedOrderRecord>& records) {
    return m_driver->restore(account, snapshot, records);
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
  std::vector<std::shared_ptr<Order>>
      OrderExecutionDriver::WrappedOrderExecutionDriver<D>::restore(
        const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot,
        const std::vector<SequencedOrderRecord>& records) {
    return m_driver->restore(account, snapshot, records);
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
