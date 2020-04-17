#include "Spire/Canvas/ValueNodes/OrderStatusNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Nexus;
using namespace Spire;
using namespace std;

OrderStatusNode::OrderStatusNode()
    : ValueNode(OrderStatus::NEW) {

    // TOSTRING TODO
  SetText(ToString(GetValue()));
}

OrderStatusNode::OrderStatusNode(OrderStatus value)
    : ValueNode(value) {

    // TOSTRING TODO
  SetText(ToString(GetValue()));
}

unique_ptr<OrderStatusNode> OrderStatusNode::SetValue(OrderStatus value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);

    // TOSTRING TODO
  clone->SetText(ToString(clone->GetValue()));
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
