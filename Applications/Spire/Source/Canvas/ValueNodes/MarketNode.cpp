#include "Spire/Canvas/ValueNodes/MarketNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

MarketNode::MarketNode()
    : ValueNode<MarketType>(MarketCode()) {
  SetText("None");
}

unique_ptr<MarketNode> MarketNode::SetValue(MarketCode value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(value.GetData());
  return clone;
}

void MarketNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> MarketNode::Clone() const {
  return make_unique<MarketNode>(*this);
}

unique_ptr<CanvasNode> MarketNode::Reset() const {
  return make_unique<MarketNode>();
}
