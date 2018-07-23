#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

CurrencyNode::CurrencyNode() {
  SetText("None");
}

CurrencyNode::CurrencyNode(CurrencyId value, string code)
    : ValueNode<CurrencyType>(value) {
  SetText(std::move(code));
}

unique_ptr<CurrencyNode> CurrencyNode::SetValue(CurrencyId value,
    string code) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(std::move(code));
  return clone;
}

void CurrencyNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> CurrencyNode::Clone() const {
  return make_unique<CurrencyNode>(*this);
}

unique_ptr<CanvasNode> CurrencyNode::Reset() const {
  return make_unique<CurrencyNode>();
}
