#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  string GetDisplayText(OrderType value) {
    if(value == OrderType::LIMIT) {
      return "Limit";
    } else if(value == OrderType::MARKET) {
      return "Market";
    } else if(value == OrderType::PEGGED) {
      return "Pegged";
    } else if(value == OrderType::STOP) {
      return "Stop";
    }
    return "None";
  }
}

OrderTypeNode::OrderTypeNode()
    : ValueNode(OrderType::LIMIT) {
  SetText(GetDisplayText(GetValue()));
}

OrderTypeNode::OrderTypeNode(OrderType value)
    : ValueNode(value) {
  SetText(GetDisplayText(GetValue()));
}

unique_ptr<OrderTypeNode> OrderTypeNode::SetValue(OrderType value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void OrderTypeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> OrderTypeNode::Clone() const {
  return make_unique<OrderTypeNode>(*this);
}

unique_ptr<CanvasNode> OrderTypeNode::Reset() const {
  return make_unique<OrderTypeNode>();
}
