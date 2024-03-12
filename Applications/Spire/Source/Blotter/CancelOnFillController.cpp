#include "Spire/Blotter/CancelOnFillController.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

CancelOnFillController::OrderEntry::OrderEntry(const Order& order)
  : m_order(&order),
    m_cancelSubmitted(false),
    m_status(OrderStatus::PENDING_NEW) {}

CancelOnFillController::CancelOnFillController(Ref<UserProfile> userProfile)
    : m_userProfile(userProfile.Get()) {
  m_slotHandler.emplace();
}

void CancelOnFillController::SetOrderExecutionPublisher(
    Ref<const OrderExecutionPublisher> orderExecutionPublisher) {
  m_slotHandler = std::nullopt;
  m_slotHandler.emplace();
  m_orderExecutionPublisher = orderExecutionPublisher.Get();
  m_orderExecutionPublisher->Monitor(m_slotHandler->GetSlot<const Order*>(
    std::bind_front(&CancelOnFillController::OnOrderExecuted, this)));
}

void CancelOnFillController::OnOrderExecuted(const Order* order) {
  auto side = order->GetInfo().m_fields.m_side;
  if(side == Side::NONE) {
    return;
  }
  auto orderEntry = std::make_shared<OrderEntry>(*order);
  auto& orderEntries =
    m_securityToOrderEntryList[order->GetInfo().m_fields.m_security][
      static_cast<int>(side)];
  orderEntries.push_back(orderEntry);
  order->GetPublisher().Monitor(m_slotHandler->GetSlot<ExecutionReport>(
    std::bind_front(&CancelOnFillController::OnExecutionReport, this,
      std::weak_ptr<OrderEntry>(orderEntry))));
}

void CancelOnFillController::OnExecutionReport(
    std::weak_ptr<OrderEntry> weakOrderEntry, const ExecutionReport& report) {
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
      orderEntry->m_order->GetInfo().m_fields.m_security)->
        is_cancel_on_fill()->get();
  if(is_cancel_on_fill) {
    auto& orderEntries = m_securityToOrderEntryList[
      orderEntry->m_order->GetInfo().m_fields.m_security][
        static_cast<int>(orderEntry->m_order->GetInfo().m_fields.m_side)];
    auto i = orderEntries.begin();
    while(i != orderEntries.end()) {
      if(i == orderEntries.begin() && IsTerminal((*i)->m_status)) {
        i = orderEntries.erase(i);
      } else if(!(*i)->m_cancelSubmitted) {
        (*i)->m_cancelSubmitted = true;
        m_userProfile->GetServiceClients().GetOrderExecutionClient().Cancel(
          *(*i)->m_order);
        ++i;
      } else {
        ++i;
      }
    }
  }
}
