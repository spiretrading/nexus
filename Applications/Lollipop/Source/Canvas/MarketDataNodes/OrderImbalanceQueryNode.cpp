#include "Spire/Canvas/MarketDataNodes/OrderImbalanceQueryNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/VenueType.hpp"
#include "Spire/Canvas/Types/OrderImbalanceRecordType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

OrderImbalanceQueryNode::OrderImbalanceQueryNode() {
  SetText("Order Imbalance Query");
  SetType(GetOrderImbalanceRecordType());
  AddChild("venue", MakeDefaultCanvasNode(VenueType::GetInstance()));
  AddChild("range", MakeDefaultCanvasNode(TimeRangeType::GetInstance()));
}

void OrderImbalanceQueryNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> OrderImbalanceQueryNode::Clone() const {
  return make_unique<OrderImbalanceQueryNode>(*this);
}
