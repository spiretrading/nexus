#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

SecurityNode::SecurityNode() {
  SetText("");
}

SecurityNode::SecurityNode(const Security& value,
    const VenueDatabase& venueDatabase)
    : ValueNode(value) {
  auto ss = std::stringstream();
  ss << venueDatabase << value;
  SetText(ss.str());
}

unique_ptr<SecurityNode> SecurityNode::SetValue(const Security& value,
    const VenueDatabase& venueDatabase) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  auto ss = std::stringstream();
  ss << venueDatabase << clone->GetValue();
  clone->SetText(ss.str());
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
