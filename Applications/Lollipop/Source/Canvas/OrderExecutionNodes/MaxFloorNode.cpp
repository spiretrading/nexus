#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include <boost/lexical_cast.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

MaxFloorNode::MaxFloorNode()
    : ValueNode(-1) {
  SetText("N/A");
}

MaxFloorNode::MaxFloorNode(Quantity value)
    : ValueNode(std::max<Quantity>(-1, value)) {
  if(value < 0) {
    SetText("N/A");
  } else {
    SetText(lexical_cast<string>(GetValue()));
  }
}

unique_ptr<MaxFloorNode> MaxFloorNode::SetValue(Quantity value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(std::max<Quantity>(-1, value));
  if(value < 0) {
    clone->SetText("N/A");
  } else {
    clone->SetText(lexical_cast<string>(clone->GetValue()));
  }
  return clone;
}

void MaxFloorNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

const string& MaxFloorNode::GetReferent() const {
  return m_referent;
}

unique_ptr<CanvasNode> MaxFloorNode::SetReferent(const string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  return std::move(clone);
}

unique_ptr<CanvasNode> MaxFloorNode::Clone() const {
  return std::make_unique<MaxFloorNode>(*this);
}

unique_ptr<CanvasNode> MaxFloorNode::Reset() const {
  return std::make_unique<MaxFloorNode>();
}
