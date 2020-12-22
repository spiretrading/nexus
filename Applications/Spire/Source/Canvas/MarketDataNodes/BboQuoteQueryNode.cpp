#include "Spire/Canvas/MarketDataNodes/BboQuoteQueryNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/QuoteRecordType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

BboQuoteQueryNode::BboQuoteQueryNode() {
  SetText("BBO Quote Query");
  SetType(GetQuoteRecordType());
  AddChild("security", MakeDefaultCanvasNode(SecurityType::GetInstance()));
  AddChild("side", MakeDefaultCanvasNode(SideType::GetInstance()));
  AddChild("range", MakeDefaultCanvasNode(TimeRangeType::GetInstance()));
}

void BboQuoteQueryNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> BboQuoteQueryNode::Clone() const {
  return make_unique<BboQuoteQueryNode>(*this);
}

BboQuoteQueryNode::BboQuoteQueryNode(ReceiveBuilder) {}
