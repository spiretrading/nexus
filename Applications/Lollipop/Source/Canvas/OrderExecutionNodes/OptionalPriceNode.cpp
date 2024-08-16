#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

OptionalPriceNode::OptionalPriceNode()
    : ValueNode(Money::ZERO),
      m_referencePrice(Money::CENT) {
  SetText("N/A");
}

OptionalPriceNode::OptionalPriceNode(Money value)
    : ValueNode(value),
      m_referencePrice(Money::CENT) {
  if(value == Money::ZERO) {
    SetText("N/A");
  } else {
    SetText(lexical_cast<string>(GetValue()));
  }
}

Money OptionalPriceNode::GetReferencePrice() const {
  return m_referencePrice;
}

unique_ptr<OptionalPriceNode> OptionalPriceNode::SetReferencePrice(
    Money referencePrice) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referencePrice = referencePrice;
  return clone;
}

unique_ptr<OptionalPriceNode> OptionalPriceNode::SetValue(Money value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  if(value == Money::ZERO) {
    clone->SetText("N/A");
  } else {
    clone->SetText(lexical_cast<string>(clone->GetValue()));
  }
  return clone;
}

void OptionalPriceNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

const string& OptionalPriceNode::GetReferent() const {
  return m_referent;
}

unique_ptr<CanvasNode> OptionalPriceNode::SetReferent(
    const string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  return std::move(clone);
}

unique_ptr<CanvasNode> OptionalPriceNode::Clone() const {
  return make_unique<OptionalPriceNode>(*this);
}

unique_ptr<CanvasNode> OptionalPriceNode::Reset() const {
  return make_unique<OptionalPriceNode>();
}
