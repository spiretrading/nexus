#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include <QString>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

MoneyNode::MoneyNode()
    : ValueNode(Money::ZERO) {
  SetText(lexical_cast<string>(GetValue()));
}

MoneyNode::MoneyNode(Money value)
    : ValueNode(value) {
  SetText(lexical_cast<string>(GetValue()));
}

unique_ptr<MoneyNode> MoneyNode::SetValue(Money value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(lexical_cast<string>(clone->GetValue()));
  return clone;
}

void MoneyNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

const string& MoneyNode::GetReferent() const {
  return m_referent;
}

unique_ptr<CanvasNode> MoneyNode::SetReferent(const string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  return std::move(clone);
}

unique_ptr<CanvasNode> MoneyNode::Clone() const {
  return make_unique<MoneyNode>(*this);
}

unique_ptr<CanvasNode> MoneyNode::Reset() const {
  return make_unique<MoneyNode>();
}
