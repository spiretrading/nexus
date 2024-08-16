#include "Spire/Canvas/MarketDataNodes/OrderImbalanceQueryNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/MarketType.hpp"
#include "Spire/Canvas/Types/OrderImbalanceRecordType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

OrderImbalanceQueryNode::OrderImbalanceQueryNode() {
  SetText("Order Imbalance Query");
  SetType(GetOrderImbalanceRecordType());
  AddChild("market", MakeDefaultCanvasNode(MarketType::GetInstance()));
  AddChild("range", MakeDefaultCanvasNode(TimeRangeType::GetInstance()));
}

void OrderImbalanceQueryNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> OrderImbalanceQueryNode::Clone() const {
  return make_unique<OrderImbalanceQueryNode>(*this);
}

OrderImbalanceQueryNode::OrderImbalanceQueryNode(ReceiveBuilder) {}
