#include "Spire/Canvas/MarketDataNodes/BboQuoteNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/QuoteRecordType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

BboQuoteNode::BboQuoteNode() {
  SetText("BBO");
  SetType(GetQuoteRecordType());
  AddChild("security", BuildDefaultCanvasNode(SecurityType::GetInstance()));
  AddChild("side", BuildDefaultCanvasNode(SideType::GetInstance()));
}

void BboQuoteNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> BboQuoteNode::Clone() const {
  return make_unique<BboQuoteNode>(*this);
}

BboQuoteNode::BboQuoteNode(ReceiveBuilder) {}
