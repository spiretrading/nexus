#include "Spire/Canvas/MarketDataNodes/TimeAndSaleQueryNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/TickerType.hpp"
#include "Spire/Canvas/Types/TimeAndSaleRecordType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

TimeAndSaleQueryNode::TimeAndSaleQueryNode() {
  SetText("Time and Sale Query");
  SetType(GetTimeAndSaleRecordType());
  AddChild("ticker", MakeDefaultCanvasNode(TickerType::GetInstance()));
  AddChild("range", MakeDefaultCanvasNode(TimeRangeType::GetInstance()));
}

void TimeAndSaleQueryNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TimeAndSaleQueryNode::Clone() const {
  return make_unique<TimeAndSaleQueryNode>(*this);
}
