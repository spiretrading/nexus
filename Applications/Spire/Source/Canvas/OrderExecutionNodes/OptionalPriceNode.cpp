#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

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
    SetText(lexical_cast<std::string>(GetValue()));
  }
}

Money OptionalPriceNode::GetReferencePrice() const {
  return m_referencePrice;
}

std::unique_ptr<OptionalPriceNode>
    OptionalPriceNode::SetReferencePrice(Money referencePrice) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referencePrice = referencePrice;
  return clone;
}

std::unique_ptr<OptionalPriceNode>
    OptionalPriceNode::SetValue(Money value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  if(value == Money::ZERO) {
    clone->SetText("N/A");
  } else {
    clone->SetText(lexical_cast<std::string>(clone->GetValue()));
  }
  return clone;
}

void OptionalPriceNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

const std::string& OptionalPriceNode::GetReferent() const {
  return m_referent;
}

std::unique_ptr<CanvasNode>
    OptionalPriceNode::SetReferent(const std::string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  return clone;
}

std::unique_ptr<CanvasNode> OptionalPriceNode::Clone() const {
  return std::make_unique<OptionalPriceNode>(*this);
}

std::unique_ptr<CanvasNode> OptionalPriceNode::Reset() const {
  return std::make_unique<OptionalPriceNode>();
}
