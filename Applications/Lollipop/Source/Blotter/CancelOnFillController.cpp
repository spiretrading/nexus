#include "Spire/Blotter/CancelOnFillController.hpp"
#include "Spire/KeyBindings/InteractionsProperties.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace std;

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
    std::bind(&CancelOnFillController::OnOrderExecuted, this,
    std::placeholders::_1)));
}

void CancelOnFillController::OnOrderExecuted(const Order* order) {
  Side side = order->GetInfo().m_fields.m_side;
  if(side == Side::NONE) {
    return;
  }
  shared_ptr<OrderEntry> orderEntry = std::make_shared<OrderEntry>(*order);
  deque<shared_ptr<OrderEntry>>& orderEntries =
    m_securityToOrderEntryList[order->GetInfo().m_fields.m_security][
    static_cast<int>(side)];
  orderEntries.push_back(orderEntry);
  order->GetPublisher().Monitor(m_slotHandler->GetSlot<ExecutionReport>(
    std::bind(&CancelOnFillController::OnExecutionReport, this,
    weak_ptr<OrderEntry>(orderEntry), std::placeholders::_1)));
}

void CancelOnFillController::OnExecutionReport(
    weak_ptr<OrderEntry> weakOrderEntry, const ExecutionReport& report) {
  shared_ptr<OrderEntry> orderEntry = weakOrderEntry.lock();
  if(orderEntry == nullptr) {
    return;
  }
  orderEntry->m_status = report.m_status;
  if(report.m_status != OrderStatus::FILLED) {
    return;
  }
  const InteractionsProperties& interactionsProperties =
    m_userProfile->GetInteractionProperties().Get(
    orderEntry->m_order->GetInfo().m_fields.m_security);
  if(interactionsProperties.m_cancelOnFill) {
    deque<shared_ptr<OrderEntry>>& orderEntries =
      m_securityToOrderEntryList[
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
