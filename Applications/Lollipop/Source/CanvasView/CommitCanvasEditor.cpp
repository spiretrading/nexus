#include "Spire/CanvasView/CommitCanvasEditor.hpp"
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimeEdit>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/ValueNodes/BooleanNode.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderStatusNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Canvas/ValueNodes/VenueNode.hpp"
#include "Spire/CanvasView/ReplaceNodeCommand.hpp"
#include "Spire/UI/MaxFloorSpinBox.hpp"
#include "Spire/UI/MoneySpinBox.hpp"
#include "Spire/UI/OptionalPriceSpinBox.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  class CommitEditorCanvasNodeVisitor : public CanvasNodeVisitor {
    public:
      CommitEditorCanvasNodeVisitor(Beam::Ref<const CanvasNode> node,
        Beam::Ref<const QWidget> editor,
        Beam::Ref<CanvasNodeModel> model,
        Beam::Ref<const UserProfile> userProfile);
      QUndoCommand* GetCommand();
      virtual void Visit(const BooleanNode& node);
      virtual void Visit(const CurrencyNode& node);
      virtual void Visit(const DecimalNode& node);
      virtual void Visit(const DestinationNode& node);
      virtual void Visit(const DurationNode& node);
      virtual void Visit(const IntegerNode& node);
      virtual void Visit(const MaxFloorNode& node);
      virtual void Visit(const MoneyNode& node);
      virtual void Visit(const OptionalPriceNode& node);
      virtual void Visit(const OrderStatusNode& node);
      virtual void Visit(const OrderTypeNode& node);
      virtual void Visit(const QueryNode& node);
      virtual void Visit(const ReferenceNode& node);
      virtual void Visit(const SideNode& node);
      virtual void Visit(const TextNode& node);
      virtual void Visit(const TimeInForceNode& node);
      virtual void Visit(const VenueNode& node);

    private:
      const CanvasNode* m_node;
      const QWidget* m_editor;
      CanvasNodeModel* m_model;
      const UserProfile* m_userProfile;
      QUndoCommand* m_command;
  };
}

QUndoCommand* Spire::CommitCanvasEditor(const CanvasNode& node,
    const QWidget& editor, CanvasNodeModel& model,
    const UserProfile& userProfile) {
  auto visitor = CommitEditorCanvasNodeVisitor(Ref(node), Ref(editor),
    Ref(model), Ref(userProfile));
  return visitor.GetCommand();
}

CommitEditorCanvasNodeVisitor::CommitEditorCanvasNodeVisitor(
    Ref<const CanvasNode> node, Ref<const QWidget> editor,
    Ref<CanvasNodeModel> model, Ref<const UserProfile> userProfile)
    : m_node(node.get()),
      m_editor(editor.get()),
      m_model(model.get()),
      m_userProfile(userProfile.get()) {}

QUndoCommand* CommitEditorCanvasNodeVisitor::GetCommand() {
  m_command = nullptr;
  m_node->Apply(*this);
  return m_command;
}

void CommitEditorCanvasNodeVisitor::Visit(const BooleanNode& node) {
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto previousValue = node.GetValue();
  bool newValue;
  if(comboEditor->currentIndex() == 0) {
    newValue = true;
  } else {
    newValue = false;
  }
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const CurrencyNode& node) {
  auto previousValue = node.GetValue();
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto& newValue = m_userProfile->GetCurrencyDatabase().from(
    comboEditor->currentText().toStdString());
  if(previousValue == newValue.m_id) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue.m_id, newValue.m_code.get_data()));
}

void CommitEditorCanvasNodeVisitor::Visit(const DecimalNode& node) {
  auto previousValue = node.GetValue();
  auto newValue = qobject_cast<const QDoubleSpinBox*>(m_editor)->value();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const DestinationNode& node) {
  auto previousValue = node.GetValue();
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto newValue = comboEditor->currentText().toStdString();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const DurationNode& node) {
  auto previousValue = node.GetValue();
  auto editorValue = qobject_cast<const QTimeEdit*>(m_editor)->time();
  auto newValue = hours(editorValue.hour()) +
    minutes(editorValue.minute()) + seconds(editorValue.second()) +
    milliseconds(editorValue.msec());
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const IntegerNode& node) {
  auto previousValue = node.GetValue();
  auto newValue = qobject_cast<const QSpinBox*>(m_editor)->value();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const MaxFloorNode& node) {
  auto previousValue = node.GetValue();
  auto newValue = dynamic_cast<const MaxFloorSpinBox*>(m_editor)->value();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const MoneyNode& node) {
  auto previousValue = node.GetValue();
  auto newValue = dynamic_cast<const MoneySpinBox*>(m_editor)->GetValue();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const OptionalPriceNode& node) {
  auto previousValue = node.GetValue();
  auto newValue = dynamic_cast<const OptionalPriceSpinBox*>(
    m_editor)->GetValue();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const OrderStatusNode& node) {
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto previousValue = node.GetValue();
  auto newValue = static_cast<OrderStatus>(comboEditor->currentIndex());
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const OrderTypeNode& node) {
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto previousValue = node.GetValue();
  OrderType newValue;
  if(comboEditor->currentIndex() == 0) {
    newValue = OrderType::LIMIT;
  } else if(comboEditor->currentIndex() == 1) {
    newValue = OrderType::MARKET;
  } else if(comboEditor->currentIndex() == 2) {
    newValue = OrderType::PEGGED;
  } else if(comboEditor->currentIndex() == 3) {
    newValue = OrderType::STOP;
  }
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const QueryNode& node) {
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto previousValue = node.GetField();
  auto newValue = comboEditor->currentText().toStdString();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetField(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const ReferenceNode& node) {
  auto previousValue = node.GetReferent();
  auto newValue = qobject_cast<const QLineEdit*>(
    m_editor)->text().toStdString();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  auto newReference = node.SetReferent(newValue);
  CanvasNodeBuilder builder(*newReference);
  builder.DeleteMetaData(*newReference, CanvasNodeModel::GetReferentKey());
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *builder.Make());
}

void CommitEditorCanvasNodeVisitor::Visit(const SideNode& node) {
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto previousValue = node.GetValue();
  Side newValue;
  if(comboEditor->currentIndex() == 0) {
    newValue = Side::BID;
  } else {
    newValue = Side::ASK;
  }
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const TextNode& node) {
  auto previousValue = node.GetValue();
  auto newValue = qobject_cast<const QLineEdit*>(
    m_editor)->text().toStdString();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const TimeInForceNode& node) {
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto previousValue = node.GetValue();
  auto newValue = TimeInForce(static_cast<TimeInForce::Type>(
    comboEditor->currentIndex()));
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue));
}

void CommitEditorCanvasNodeVisitor::Visit(const VenueNode& node) {
  auto previousValue = node.GetValue();
  auto comboEditor = qobject_cast<const QComboBox*>(m_editor);
  auto& newValue = m_userProfile->GetVenueDatabase().from(
    comboEditor->currentText().toStdString());
  if(previousValue == newValue.m_venue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_command = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetValue(newValue.m_venue));
}
