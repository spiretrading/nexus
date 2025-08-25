#ifndef NEXUS_MANUAL_ORDER_ENTRY_DRIVER_HPP
#define NEXUS_MANUAL_ORDER_ENTRY_DRIVER_HPP
#include <utility>
#include <Beam/Collections/SynchronizedList.hpp>
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
  template<typename D, typename A>
  class ManualOrderEntryDriver {
    public:

      /**
       * The type of OrderExecutionDriver to send non-manual Order entries to.
       */
      using OrderExecutionDriver = Beam::GetTryDereferenceType<D>;

      /**
       * The type of AdministrationClient used to authorize the use of a manual
       * order entry.
       */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /**
       * Constructs a ManualOrderEntryDriver.
       * @param destination The destination used to trigger a manual Order.
       * @param driver The OrderExecutionDriver to send non-manual Order entries
       *        to.
       * @param administration_client Initializes the AdministrationClient.
       */
      template<typename DF, typename AF>
      ManualOrderEntryDriver(std::string destination, DF&& driver,
        AF&& administration_client);
      ~ManualOrderEntryDriver();
      std::shared_ptr<const Order> recover(
        const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<const Order>& order);
      std::shared_ptr<const Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      std::string m_destination;
      Beam::GetOptionalLocalPtr<D> m_driver;
      Beam::GetOptionalLocalPtr<A> m_administration_client;
      Beam::SynchronizedUnorderedSet<OrderId> m_ids;
      Beam::IO::OpenState m_open_state;

      ManualOrderEntryDriver(const ManualOrderEntryDriver&) = delete;
      ManualOrderEntryDriver& operator =(
        const ManualOrderEntryDriver&) = delete;
  };

  template<typename DF, typename AF>
  ManualOrderEntryDriver(std::string, DF&&, AF&&) ->
    ManualOrderEntryDriver<std::decay_t<DF>, std::decay_t<AF>>;

  template<typename D, typename A>
  template<typename DF, typename AF>
  ManualOrderEntryDriver<D, A>::ManualOrderEntryDriver(
    std::string destination, DF&& driver, AF&& administration_client)
    : m_destination(std::move(destination)),
      m_driver(std::forward<DF>(driver)),
      m_administration_client(std::forward<AF>(administration_client)) {}

  template<typename D, typename A>
  ManualOrderEntryDriver<D, A>::~ManualOrderEntryDriver() {
    close();
  }

  template<typename D, typename A>
  std::shared_ptr<const Order> ManualOrderEntryDriver<D, A>::recover(
      const SequencedAccountOrderRecord& record) {
    if((*record)->m_info.m_fields.m_destination == m_destination) {
      auto order = std::make_shared<PrimitiveOrder>(**record);
      m_ids.Insert(order->get_info().m_id);
      return order;
    } else {
      return m_driver->recover(record);
    }
  }

  template<typename D, typename A>
  void ManualOrderEntryDriver<D, A>::add(
      const std::shared_ptr<const Order>& order) {
    m_driver->add(order);
  }

  template<typename D, typename A>
  std::shared_ptr<const Order> ManualOrderEntryDriver<D, A>::submit(
      const OrderInfo& info) {
    if(info.m_fields.m_destination != m_destination) {
      return m_driver->submit(info);
    }
    auto is_administrator =
      m_administration_client->check_administrator(info.m_submission_account);
    if(!is_administrator) {
      auto order = make_rejected_order(
        info, "Insufficient permissions to execute a manual order.");
      m_ids.Insert(order->get_info().m_id);
      return order;
    }
    auto order = std::make_shared<PrimitiveOrder>(info);
    order->with([&] (auto status, const auto& reports) {
      auto& last_report = reports.back();
      auto updated_report =
        make_update(last_report, OrderStatus::NEW, info.m_timestamp);
      order->update(updated_report);
    });
    order->with([&] (auto status, const auto& reports) {
      auto& last_report = reports.back();
      auto updated_report =
        make_update(last_report, OrderStatus::FILLED, info.m_timestamp);
      updated_report.m_last_quantity = order->get_info().m_fields.m_quantity;
      updated_report.m_last_price = order->get_info().m_fields.m_price;
      updated_report.m_last_market = m_destination;
      order->update(updated_report);
    });
    m_ids.Insert(order->get_info().m_id);
    return order;
  }

  template<typename D, typename A>
  void ManualOrderEntryDriver<D, A>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    if(m_ids.Contains(id)) {
      return;
    }
    return m_driver->cancel(session, id);
  }

  template<typename D, typename A>
  void ManualOrderEntryDriver<D, A>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    if(m_ids.Contains(id)) {
      return;
    }
    return m_driver->Update(session, id, report);
  }

  template<typename D, typename A>
  void ManualOrderEntryDriver<D, A>::close() {
    m_open_state.Close();
  }
}

#endif
