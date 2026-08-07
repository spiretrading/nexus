#ifndef NEXUS_MANUAL_ORDER_ENTRY_DRIVER_HPP
#define NEXUS_MANUAL_ORDER_ENTRY_DRIVER_HPP
#include <cstddef>
#include <utility>
#include <vector>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {

  /**
   * Allows a position to be manually assigned to an account.
   * @param <D> The type of OrderExecutionDriver to send non-manual Order
   *        entries to.
   * @param <A> The type of AdministrationClient used to authorize the use of a
   *        manual order entry.
   */
  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  class ManualOrderEntryDriver {
    public:

      /**
       * The type of OrderExecutionDriver to send non-manual Order entries to.
       */
      using OrderExecutionDriver = Beam::dereference_t<D>;

      /**
       * The type of AdministrationClient used to authorize the use of a manual
       * order entry.
       */
      using AdministrationClient = Beam::dereference_t<A>;

      /**
       * Constructs a ManualOrderEntryDriver.
       * @param destination The destination used to trigger a manual Order.
       * @param driver The OrderExecutionDriver to send non-manual Order entries
       *        to.
       * @param administration_client Initializes the AdministrationClient.
       */
      template<Beam::Initializes<D> DF, Beam::Initializes<A> AF>
      ManualOrderEntryDriver(
        std::string destination, DF&& driver, AF&& administration_client);

      ~ManualOrderEntryDriver();

      std::vector<std::shared_ptr<Order>> restore(
        const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot,
        const std::vector<SequencedOrderRecord>& records);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      std::string m_destination;
      Beam::local_ptr_t<D> m_driver;
      Beam::local_ptr_t<A> m_administration_client;
      Beam::SynchronizedUnorderedSet<OrderId> m_ids;
      Beam::OpenState m_open_state;

      void fill(const std::shared_ptr<PrimitiveOrder>& order);
      ManualOrderEntryDriver(const ManualOrderEntryDriver&) = delete;
      ManualOrderEntryDriver& operator =(
        const ManualOrderEntryDriver&) = delete;
  };

  template<typename D, typename A>
  ManualOrderEntryDriver(std::string, D&&, A&&) ->
    ManualOrderEntryDriver<std::remove_cvref_t<D>, std::remove_cvref_t<A>>;

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<Beam::Initializes<D> DF, Beam::Initializes<A> AF>
  ManualOrderEntryDriver<D, A>::ManualOrderEntryDriver(
    std::string destination, DF&& driver, AF&& administration_client)
    : m_destination(std::move(destination)),
      m_driver(std::forward<DF>(driver)),
      m_administration_client(std::forward<AF>(administration_client)) {}

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  ManualOrderEntryDriver<D, A>::~ManualOrderEntryDriver() {
    close();
  }

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  std::vector<std::shared_ptr<Order>> ManualOrderEntryDriver<D, A>::restore(
      const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot,
      const std::vector<SequencedOrderRecord>& records) {
    auto orders = std::vector<std::shared_ptr<Order>>(records.size());
    auto forwarded_records = std::vector<SequencedOrderRecord>();
    auto forwarded_indices = std::vector<std::size_t>();
    for(auto i = std::size_t(0); i != records.size(); ++i) {
      auto& record = records[i];
      if(record->m_info.m_fields.m_destination != m_destination) {
        forwarded_records.push_back(record);
        forwarded_indices.push_back(i);
        continue;
      }
      auto order = [&] {
        if(record->m_execution_reports.empty()) {
          return std::make_shared<PrimitiveOrder>(record->m_info);
        }
        return std::make_shared<PrimitiveOrder>(*record);
      }();
      fill(order);
      m_ids.insert(record->m_info.m_id);
      orders[i] = std::move(order);
    }
    auto forwarded_orders =
      m_driver->restore(account, snapshot, forwarded_records);
    for(auto i = std::size_t(0); i != forwarded_orders.size(); ++i) {
      orders[forwarded_indices[i]] = std::move(forwarded_orders[i]);
    }
    return orders;
  }

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void ManualOrderEntryDriver<D, A>::add(const std::shared_ptr<Order>& order) {
    m_driver->add(order);
  }

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  std::shared_ptr<Order> ManualOrderEntryDriver<D, A>::submit(
      const OrderInfo& info) {
    if(info.m_fields.m_destination != m_destination) {
      return m_driver->submit(info);
    }
    auto is_administrator =
      m_administration_client->check_administrator(info.m_submission_account);
    if(!is_administrator) {
      auto order = make_rejected_order(
        info, "Insufficient permissions to execute a manual order.");
      m_ids.insert(order->get_info().m_id);
      return order;
    }
    auto order = std::make_shared<PrimitiveOrder>(info);
    fill(order);
    m_ids.insert(order->get_info().m_id);
    m_driver->add(order);
    return order;
  }

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void ManualOrderEntryDriver<D, A>::fill(
      const std::shared_ptr<PrimitiveOrder>& order) {
    order->with([&] (auto status, const auto& reports) {
      if(is_terminal(status)) {
        return;
      }
      if(status == OrderStatus::PENDING_NEW) {
        order->update(make_update(
          reports.back(), OrderStatus::NEW, order->get_info().m_timestamp));
      }
    });
    order->with([&] (auto status, const auto& reports) {
      if(is_terminal(status)) {
        return;
      }
      auto report = make_update(
        reports.back(), OrderStatus::FILLED, order->get_info().m_timestamp);
      report.m_last_quantity = order->get_info().m_fields.m_quantity;
      report.m_last_price = order->get_info().m_fields.m_price;
      report.m_last_market = m_destination;
      order->update(report);
    });
  }

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void ManualOrderEntryDriver<D, A>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    if(m_ids.contains(id)) {
      return;
    }
    return m_driver->cancel(session, id);
  }

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void ManualOrderEntryDriver<D, A>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    if(m_ids.contains(id)) {
      return;
    }
    return m_driver->update(session, id, report);
  }

  template<typename D, typename A> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void ManualOrderEntryDriver<D, A>::close() {
    m_open_state.close();
  }
}

#endif
