#include "Spire/CanvasView/OrderTaskView.hpp"
#include <Beam/Utilities/Algorithm.hpp>
#include <QKeyEvent>
#include <QMessageBox>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include "Spire/CanvasView/CanvasNodeNotVisibleException.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

OrderTaskView::OrderTaskView(const DisplayWidgetSlot& displayWidgetSlot,
    const RemoveWidgetSlot& removeWidgetSlot,  Ref<QWidget> parent,
    Ref<UserProfile> userProfile)
    : m_parent{parent.get()},
      m_userProfile{userProfile.get()},
      m_isTaskEntryWidgetForInteractionsProperties{false},
      m_taskEntryWidget{nullptr},
      m_displayWidgetSlot{displayWidgetSlot},
      m_removeWidgetSlot{removeWidgetSlot} {}

bool OrderTaskView::HandleKeyPressEvent(const QKeyEvent& event,
    const Security& security, Money askPrice, Money bidPrice) {
  State state;
  state.m_security = &security;
  state.m_askPrice = &askPrice;
  state.m_bidPrice = &bidPrice;
  m_state = &state;
  if(m_taskEntryWidget != nullptr) {
    return HandleTaskInputEvent(event);
  } else if(security != Security{}) {
    auto taskBinding = m_userProfile->GetKeyBindings().GetTaskFromBinding(
      security.get_venue(), event.key());
    if(taskBinding.is_initialized()) {
      return HandleKeyBindingEvent(*taskBinding);
    }
    auto cancelBinding = m_userProfile->GetKeyBindings().GetCancelFromBinding(
      event.key());
    if(cancelBinding.is_initialized()) {
      return  HandleCancelBindingEvent(*cancelBinding);
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
  auto errors = Validate(node);
  if(!errors.empty()) {
    QMessageBox::warning(m_parent, QObject::tr("Error"),
      QString::fromStdString(errors.front().GetErrorMessage()));
    return;
  }
  auto& activeBlotter = m_userProfile->GetBlotterSettings().GetActiveBlotter();
  auto& blotterWindow = BlotterWindow::GetBlotterWindow(Ref(*m_userProfile),
    Ref(activeBlotter));
  if(!blotterWindow.isVisible()) {
    blotterWindow.setAttribute(Qt::WA_ShowWithoutActivating);
    blotterWindow.show();
    m_parent->raise();
  }
  auto& entry = activeBlotter.GetTasksModel().Add(node);
  m_tasksExecuted[*m_state->m_security].push_back(entry.m_task);
  entry.m_task->GetPublisher().monitor(m_slotHandler.get_slot<Task::StateEntry>(
    [=, security = *m_state->m_security, task = entry.m_task] (
        const Task::StateEntry& update) {
      OnTaskState(task, security, update);
    }));
  entry.m_task->Execute();
}

unique_ptr<CanvasNode> OrderTaskView::InitializeTaskNode(
    const CanvasNode& baseNode) {
  auto taskNode = CanvasNode::Clone(baseNode);
  auto securityNode = taskNode->FindNode(
    SingleOrderTaskNode::SECURITY_PROPERTY);
  if(securityNode.is_initialized() && !securityNode->IsReadOnly()) {
    if(auto securityValueNode =
        dynamic_cast<const SecurityNode*>(&*securityNode)) {
      CanvasNodeBuilder builder{*taskNode};
      builder.Replace(*securityNode, securityValueNode->SetValue(
        *m_state->m_security, m_userProfile->GetVenueDatabase()));
      builder.SetReadOnly(*securityNode, true);
      auto sideNode = taskNode->FindNode(SingleOrderTaskNode::SIDE_PROPERTY);
      auto price = [&] {
        if(sideNode.is_initialized()) {
          if(auto sideValueNode = dynamic_cast<const SideNode*>(&*sideNode)) {
            if(sideValueNode->GetValue() == Side::BID) {
              return *m_state->m_bidPrice;
            } else {
              return *m_state->m_askPrice;
            }
          } else {
            return *m_state->m_bidPrice;
          }
        } else {
          return *m_state->m_askPrice;
        }
      }();
      auto priceNode = taskNode->FindNode(SingleOrderTaskNode::PRICE_PROPERTY);
      if(priceNode.is_initialized() && !priceNode->IsReadOnly()) {
        if(auto moneyNode = dynamic_cast<const MoneyNode*>(&*priceNode)) {
          builder.Replace(*priceNode, moneyNode->SetValue(price));
        } else if(auto moneyNode =
            dynamic_cast<const OptionalPriceNode*>(&*priceNode)) {
          builder.Replace(*priceNode, moneyNode->SetReferencePrice(price));
        }
      }
      auto quantityNode = taskNode->FindNode(
        SingleOrderTaskNode::QUANTITY_PROPERTY);
      if(quantityNode.is_initialized() && !quantityNode->IsReadOnly()) {
        if(auto quantityValueNode = dynamic_cast<const IntegerNode*>(
            &*quantityNode)) {
          auto quantity = [&] {
            auto sideNode = taskNode->FindNode(
              SingleOrderTaskNode::SIDE_PROPERTY);
            if(sideNode.is_initialized()) {
              if(auto sideValueNode = dynamic_cast<const SideNode*>(
                  &*sideNode)) {
                return m_userProfile->GetDefaultQuantity(*m_state->m_security,
                  sideValueNode->GetValue());
              } else {
                return m_userProfile->GetInteractionProperties().get(
                  *m_state->m_security).m_defaultQuantity;
              }
            } else {
              return m_userProfile->GetInteractionProperties().get(
                *m_state->m_security).m_defaultQuantity;
            }
          }();
          builder.Replace(*quantityNode, quantityValueNode->SetValue(
            static_cast<int>(quantity)));
        }
      }
      taskNode = builder.Make();
    }
  }
  return taskNode;
}

bool OrderTaskView::HandleKeyBindingEvent(
    const KeyBindings::TaskBinding& keyBinding) {
  auto taskNode = InitializeTaskNode(*keyBinding.m_node);
  m_taskEntryWidget = new CondensedCanvasWidget{keyBinding.m_name,
    Ref(*m_userProfile), m_parent};
  auto coordinate = CanvasNodeModel::Coordinate{0, 0};
  auto isVisible = [&] {
    try {
      m_taskEntryWidget->Add(coordinate, *taskNode);
      return true;
    } catch(const CanvasNodeNotVisibleException&) {
      return false;
    }
  }();
  if(isVisible) {
    m_taskEntryWidget->setSizePolicy(QSizePolicy::Preferred,
      QSizePolicy::Fixed);
    m_displayWidgetSlot(*m_taskEntryWidget);
    return true;
  } else {
    m_taskEntryWidget->deleteLater();
    m_taskEntryWidget = nullptr;
    ExecuteTask(*taskNode);
    return true;
  }
}

bool OrderTaskView::HandleTaskInputEvent(const QKeyEvent& event) {
  auto baseKey = event.key();
  if(baseKey == Qt::Key_Escape) {
    RemoveTaskEntry();
    return true;
  } else if(baseKey == Qt::Key_Enter || baseKey == Qt::Key_Return) {
    if(m_isTaskEntryWidgetForInteractionsProperties) {
      auto& interactionsNode = static_cast<const InteractionsNode&>(
        *m_taskEntryWidget->GetRoots().front());
      m_userProfile->GetInteractionProperties().set(*m_state->m_security,
        interactionsNode.GetProperties());
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
    auto key = QKeySequence{static_cast<int>(event.modifiers() + event.key())};
    auto taskBinding = m_userProfile->GetKeyBindings().GetTaskFromBinding(
      m_state->m_security->get_venue(), key);
    if(taskBinding.is_initialized()) {
      RemoveTaskEntry();
      return HandleKeyBindingEvent(*taskBinding);
    }
  }
  return false;
}

void OrderTaskView::HandleInteractionsPropertiesEvent() {
  auto& interactionsProperties =
    m_userProfile->GetInteractionProperties().get(*m_state->m_security);
  auto interactionsNode = std::make_unique<InteractionsNode>(
    *m_state->m_security, m_userProfile->GetVenueDatabase(),
    interactionsProperties);
  m_taskEntryWidget = new CondensedCanvasWidget{"Interactions",
    Ref(*m_userProfile), m_parent};
  m_isTaskEntryWidgetForInteractionsProperties = true;
  auto coordinate = CanvasNodeModel::Coordinate{0, 0};
  m_taskEntryWidget->Add(coordinate, *interactionsNode);
  m_taskEntryWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_displayWidgetSlot(*m_taskEntryWidget);
}

bool OrderTaskView::HandleCancelBindingEvent(
    const KeyBindings::CancelBinding& keyBinding) {
  flush(m_slotHandler);
  KeyBindings::CancelBinding::HandleCancel(keyBinding,
    out(m_tasksExecuted[*m_state->m_security]));
  return true;
}

void OrderTaskView::OnTaskState(const std::shared_ptr<Task>& task,
    const Security& security, const Task::StateEntry& update) {
  if(IsTerminal(update.m_state)) {
    remove_first(m_tasksExecuted[security], task);
  }
}
