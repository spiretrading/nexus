#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

TickerNode::TickerNode() {
  SetText("");
}

TickerNode::TickerNode(const Ticker& value, const VenueDatabase& venueDatabase)
    : ValueNode(value) {
  SetText(displayText(value).toStdString());
}

unique_ptr<TickerNode> TickerNode::SetValue(const Ticker& value,
    const VenueDatabase& venueDatabase) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(displayText(clone->GetValue()).toStdString());
  return clone;
}

void TickerNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TickerNode::Clone() const {
  return make_unique<TickerNode>(*this);
}

unique_ptr<CanvasNode> TickerNode::Reset() const {
  return make_unique<TickerNode>();
}
