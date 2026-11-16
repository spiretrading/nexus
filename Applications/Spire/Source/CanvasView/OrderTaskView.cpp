#include "Spire/CanvasView/OrderTaskView.hpp"
#include <QKeyEvent>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include "Spire/CanvasView/CanvasNodeNotVisibleException.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

OrderTaskView::OrderTaskView(const DisplayWidgetSlot& displayWidgetSlot,
  const RemoveWidgetSlot& removeWidgetSlot, Ref<QWidget> parent,
  Ref<UserProfile> userProfile)
  : m_parent(parent.get()),
    m_userProfile(userProfile.get()),
    m_isTaskEntryWidgetForInteractionsProperties(false),
    m_taskEntryWidget(nullptr),
    m_displayWidgetSlot(displayWidgetSlot),
    m_removeWidgetSlot(removeWidgetSlot) {}

bool OrderTaskView::HandleKeyPressEvent(const QKeyEvent& event,
    const Ticker& ticker, Money askPrice, Money bidPrice) {
  auto state = State();
  state.m_ticker = &ticker;
  state.m_askPrice = &askPrice;
  state.m_bidPrice = &bidPrice;
  m_state = &state;
  if(m_taskEntryWidget) {
    return HandleTaskInputEvent(event);
  } else if(ticker) {
    auto& keyBindings = *m_userProfile->GetKeyBindings();
    auto taskBinding =
      find_order_task_arguments(*keyBindings.get_order_task_arguments(),
        ticker.get_venue(), event.key());
    if(taskBinding) {
      return HandleKeyBindingEvent(*taskBinding);
    }
    auto cancelBinding =
      keyBindings.get_cancel_key_bindings()->find_operation(event.key());
    if(cancelBinding) {
      return HandleCancelBindingEvent(*cancelBinding);
    }
  }
  return false;
}

void OrderTaskView::RemoveTaskEntry() {
  m_parent->setUpdatesEnabled(false);
  m_removeWidgetSlot(*m_taskEntryWidget);
  m_taskEntryWidget->deleteLater();
  m_taskEntryWidget = nullptr;
  m_isTaskEntryWidgetForInteractionsProperties = false;
  m_parent->setUpdatesEnabled(true);
}

void OrderTaskView::ExecuteTask(const CanvasNode& node) {
  // TODO: Submit the task through the new active blotter.
}

std::unique_ptr<CanvasNode>
    OrderTaskView::InitializeTaskNode(const CanvasNode& baseNode) {
  auto taskNode = CanvasNode::Clone(baseNode);
  auto tickerNode = taskNode->FindNode(SingleOrderTaskNode::TICKER_PROPERTY);
  if(tickerNode && !tickerNode->IsReadOnly()) {
    if(auto tickerValueNode = dynamic_cast<const TickerNode*>(&*tickerNode)) {
      auto builder = CanvasNodeBuilder(*taskNode);
      builder.Replace(
        *tickerNode, tickerValueNode->SetValue(*m_state->m_ticker));
      builder.SetReadOnly(*tickerNode, true);
      auto sideNode = taskNode->FindNode(SingleOrderTaskNode::SIDE_PROPERTY);
      auto price = [&] {
        if(sideNode) {
          if(auto sideValueNode = dynamic_cast<const SideNode*>(&*sideNode)) {
            if(sideValueNode->GetValue() == Side::ASK) {
              return *m_state->m_askPrice;
            }
          }
        }
        return *m_state->m_bidPrice;
      }();
      auto priceNode = taskNode->FindNode(SingleOrderTaskNode::PRICE_PROPERTY);
      if(priceNode && !priceNode->IsReadOnly()) {
        if(auto moneyNode = dynamic_cast<const MoneyNode*>(&*priceNode)) {
          builder.Replace(*priceNode, moneyNode->SetValue(price));
        } else if(auto moneyNode =
            dynamic_cast<const OptionalPriceNode*>(&*priceNode)) {
          builder.Replace(*priceNode, moneyNode->SetReferencePrice(price));
        }
      }
      auto quantityNode =
        taskNode->FindNode(SingleOrderTaskNode::QUANTITY_PROPERTY);
      if(quantityNode && !quantityNode->IsReadOnly()) {
        if(auto quantityValueNode =
            dynamic_cast<const IntegerNode*>(&*quantityNode)) {
          auto quantity = [&] {
            auto sideNode =
              taskNode->FindNode(SingleOrderTaskNode::SIDE_PROPERTY);
            auto& interactions =
              *m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
                *m_state->m_ticker);
            if(sideNode) {
              if(auto sideValueNode =
                  dynamic_cast<const SideNode*>(&*sideNode)) {
                return get_default_order_quantity(*m_userProfile,
                  *m_state->m_ticker, sideValueNode->GetValue());
              }
            }
            return interactions.get_default_quantity()->get();
          }();
          builder.Replace(*quantityNode,
            quantityValueNode->SetValue(static_cast<int>(quantity)));
        }
      }
      taskNode = builder.Make();
    }
  }
  return taskNode;
}

bool OrderTaskView::HandleKeyBindingEvent(const OrderTaskArguments& arguments) {
  auto taskNode = InitializeTaskNode(
    *make_canvas_node(arguments, m_userProfile->GetAdditionalTagDatabase()));
  m_taskEntryWidget = new CondensedCanvasWidget(
    arguments.m_name.toStdString(), Ref(*m_userProfile), m_parent);
  auto coordinate = CanvasNodeModel::Coordinate(0, 0);
  auto isVisible = [&] {
    try {
      m_taskEntryWidget->Add(coordinate, *taskNode);
      return true;
    } catch(const CanvasNodeNotVisibleException&) {
      return false;
    }
  }();
  if(isVisible) {
    m_taskEntryWidget->setSizePolicy(
      QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_displayWidgetSlot(*m_taskEntryWidget);
    return true;
  }
  m_taskEntryWidget->deleteLater();
  m_taskEntryWidget = nullptr;
  ExecuteTask(*taskNode);
  return true;
}

bool OrderTaskView::HandleTaskInputEvent(const QKeyEvent& event) {
  auto baseKey = event.key();
  if(baseKey == Qt::Key_Escape) {
    RemoveTaskEntry();
    return true;
  } else if(baseKey == Qt::Key_Enter || baseKey == Qt::Key_Return) {
    if(m_isTaskEntryWidgetForInteractionsProperties) {
      auto& node = static_cast<const InteractionsNode&>(
        *m_taskEntryWidget->GetRoots().front());
      apply(node, *m_userProfile->GetKeyBindings());
      RemoveTaskEntry();
      return true;
    } else {
      auto taskNode = CanvasNode::Clone(*m_taskEntryWidget->GetRoots().front());
      RemoveTaskEntry();
      ExecuteTask(*taskNode);
      return true;
    }
  } else if(baseKey == Qt::Key_QuoteLeft) {
    RemoveTaskEntry();
    HandleInteractionsPropertiesEvent();
    return true;
  } else {
    auto key = QKeySequence(static_cast<int>(event.modifiers() + event.key()));
    auto arguments = find_order_task_arguments(
      *m_userProfile->GetKeyBindings()->get_order_task_arguments(),
      *m_state->m_ticker, key);
    if(arguments) {
      RemoveTaskEntry();
      return HandleKeyBindingEvent(*arguments);
    }
  }
  return false;
}

void OrderTaskView::HandleInteractionsPropertiesEvent() {
  auto& interactions =
    *m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      *m_state->m_ticker);
  auto interactionsNode = std::make_unique<InteractionsNode>(
    *m_state->m_ticker, interactions);
  m_taskEntryWidget =
    new CondensedCanvasWidget("Interactions", Ref(*m_userProfile), m_parent);
  m_isTaskEntryWidgetForInteractionsProperties = true;
  auto coordinate = CanvasNodeModel::Coordinate(0, 0);
  m_taskEntryWidget->Add(coordinate, *interactionsNode);
  m_taskEntryWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_displayWidgetSlot(*m_taskEntryWidget);
}

bool OrderTaskView::HandleCancelBindingEvent(
    const CancelKeyBindingsModel::Operation& operation) {
  // TODO: Cancel tasks through the new active blotter.
  return true;
}
