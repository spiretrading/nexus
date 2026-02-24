#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

CurrencyNode::CurrencyNode() {
  SetText("None");
}

CurrencyNode::CurrencyNode(Asset value)
    : ValueNode<CurrencyType>(value) {
  SetText(displayText(value).toStdString());
}

unique_ptr<CurrencyNode> CurrencyNode::SetValue(Asset value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(displayText(value).toStdString());
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
