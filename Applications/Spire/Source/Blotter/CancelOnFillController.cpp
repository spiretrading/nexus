#include "Spire/Blotter/CancelOnFillController.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

CancelOnFillController::OrderEntry::OrderEntry(
  const std::shared_ptr<Order>& order)
  : m_order(order),
    m_cancelSubmitted(false),
    m_status(OrderStatus::PENDING_NEW) {}

CancelOnFillController::CancelOnFillController(Ref<UserProfile> userProfile)
    : m_userProfile(userProfile.get()) {
  m_slotHandler.emplace();
}

void CancelOnFillController::SetOrderExecutionPublisher(Ref<
    const Publisher<std::shared_ptr<Order>>> orderExecutionPublisher) {
  m_slotHandler = std::nullopt;
  m_slotHandler.emplace();
  m_orderExecutionPublisher = orderExecutionPublisher.get();
  m_orderExecutionPublisher->monitor(
    m_slotHandler->get_slot<std::shared_ptr<Order>>(
      std::bind(&CancelOnFillController::OnOrderExecuted, this,
        std::placeholders::_1)));
}

void CancelOnFillController::OnOrderExecuted(
    const std::shared_ptr<Order>& order) {
  Side side = order->get_info().m_fields.m_side;
  if(side == Side::NONE) {
    return;
  }
  shared_ptr<OrderEntry> orderEntry = std::make_shared<OrderEntry>(order);
  deque<shared_ptr<OrderEntry>>& orderEntries =
    m_securityToOrderEntryList[order->get_info().m_fields.m_security][
    static_cast<int>(side)];
  orderEntries.push_back(orderEntry);
  order->get_publisher().monitor(m_slotHandler->get_slot<ExecutionReport>(
    std::bind(&CancelOnFillController::OnExecutionReport, this,
    weak_ptr<OrderEntry>(orderEntry), std::placeholders::_1)));
}

void CancelOnFillController::OnExecutionReport(
    weak_ptr<OrderEntry> weakOrderEntry, const ExecutionReport& report) {
  auto orderEntry = weakOrderEntry.lock();
  if(!orderEntry) {
    return;
  }
  orderEntry->m_status = report.m_status;
  if(report.m_status != OrderStatus::FILLED) {
    return;
  }
  auto is_cancel_on_fill =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      orderEntry->m_order->get_info().m_fields.m_security)->
        is_cancel_on_fill()->get();
  if(is_cancel_on_fill) {
    auto& orderEntries = m_securityToOrderEntryList[
      orderEntry->m_order->get_info().m_fields.m_security][
        static_cast<int>(orderEntry->m_order->get_info().m_fields.m_side)];
    auto i = orderEntries.begin();
    while(i != orderEntries.end()) {
      if(i == orderEntries.begin() && is_terminal((*i)->m_status)) {
        i = orderEntries.erase(i);
      } else if(!(*i)->m_cancelSubmitted) {
        (*i)->m_cancelSubmitted = true;
        m_userProfile->GetClients().get_order_execution_client().cancel(
          *(*i)->m_order);
        ++i;
      } else {
        ++i;
      }
    }
  }
}
