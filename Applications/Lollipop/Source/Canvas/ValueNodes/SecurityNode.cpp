#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

SecurityNode::SecurityNode() {
  SetText("");
}

SecurityNode::SecurityNode(const Security& value,
    const VenueDatabase& venueDatabase)
    : ValueNode(value) {
  SetText(displayText(value).toStdString());
}

unique_ptr<SecurityNode> SecurityNode::SetValue(const Security& value,
    const VenueDatabase& venueDatabase) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(displayText(clone->GetValue()).toStdString());
  return clone;
}

void SecurityNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> SecurityNode::Clone() const {
  return make_unique<SecurityNode>(*this);
}

unique_ptr<CanvasNode> SecurityNode::Reset() const {
  return make_unique<SecurityNode>();
}
