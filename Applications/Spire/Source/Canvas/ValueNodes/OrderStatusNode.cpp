#include "Spire/Canvas/ValueNodes/OrderStatusNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

OrderStatusNode::OrderStatusNode()
    : ValueNode(OrderStatus::NEW) {
  SetText(displayText(GetValue()).toStdString());
}

OrderStatusNode::OrderStatusNode(OrderStatus value)
    : ValueNode(value) {
  SetText(displayText(GetValue()).toStdString());
}

unique_ptr<OrderStatusNode> OrderStatusNode::SetValue(OrderStatus value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(displayText(clone->GetValue()).toStdString());
  return clone;
}

void OrderStatusNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> OrderStatusNode::Clone() const {
  return make_unique<OrderStatusNode>(*this);
}

unique_ptr<CanvasNode> OrderStatusNode::Reset() const {
  return make_unique<OrderStatusNode>();
}
