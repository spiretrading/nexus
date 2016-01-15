#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include <QString>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

DecimalNode::DecimalNode()
      : ValueNode(0) {
  SetText(QString::number(GetValue()).toStdString());
}

DecimalNode::DecimalNode(double value)
    : ValueNode(value) {
  SetText(QString::number(GetValue()).toStdString());
}

unique_ptr<DecimalNode> DecimalNode::SetValue(double value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(QString::number(clone->GetValue()).toStdString());
  return clone;
}

void DecimalNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> DecimalNode::Clone() const {
  return make_unique<DecimalNode>(*this);
}

unique_ptr<CanvasNode> DecimalNode::Reset() const {
  return unique_ptr<DecimalNode>();
}
