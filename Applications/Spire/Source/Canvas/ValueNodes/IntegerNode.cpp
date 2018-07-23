#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include <boost/lexical_cast.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

IntegerNode::IntegerNode() {
  SetText(lexical_cast<string>(GetValue()));
}

IntegerNode::IntegerNode(Quantity value)
    : ValueNode(value) {
  SetText(lexical_cast<string>(GetValue()));
}

unique_ptr<IntegerNode> IntegerNode::SetValue(int value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(lexical_cast<string>(clone->GetValue()));
  return clone;
}

void IntegerNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

const string& IntegerNode::GetReferent() const {
  return m_referent;
}

unique_ptr<CanvasNode> IntegerNode::SetReferent(const string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  return std::move(clone);
}

unique_ptr<CanvasNode> IntegerNode::Clone() const {
  return make_unique<IntegerNode>(*this);
}

unique_ptr<CanvasNode> IntegerNode::Reset() const {
  return make_unique<IntegerNode>();
}
