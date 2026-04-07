#include "Spire/Canvas/ValueNodes/VenueNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

VenueNode::VenueNode()
    : ValueNode<VenueType>(Venue()) {
  SetText("None");
}

unique_ptr<VenueNode> VenueNode::SetValue(Venue value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(lexical_cast<std::string>(value));
  return clone;
}

void VenueNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> VenueNode::Clone() const {
  return std::make_unique<VenueNode>(*this);
}

unique_ptr<CanvasNode> VenueNode::Reset() const {
  return make_unique<VenueNode>();
}
