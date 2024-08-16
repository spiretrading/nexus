#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include <boost/lexical_cast.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

IntegerNode::IntegerNode() {
  SetText(lexical_cast<std::string>(GetValue()));
}

IntegerNode::IntegerNode(Quantity value)
    : ValueNode(value) {
  SetText(lexical_cast<std::string>(GetValue()));
}

std::unique_ptr<IntegerNode> IntegerNode::SetValue(int value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(lexical_cast<std::string>(clone->GetValue()));
  return clone;
}

void IntegerNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

const std::string& IntegerNode::GetReferent() const {
  return m_referent;
}

std::unique_ptr<CanvasNode>
    IntegerNode::SetReferent(const std::string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  return clone;
}

std::unique_ptr<CanvasNode> IntegerNode::Clone() const {
  return std::make_unique<IntegerNode>(*this);
}

std::unique_ptr<CanvasNode> IntegerNode::Reset() const {
  return std::make_unique<IntegerNode>();
}
