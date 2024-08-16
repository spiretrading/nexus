#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Spire;
using namespace std;

NoneNode::NoneNode() {
  SetText("None");
  SetType(UnionType::GetAnyType());
}

NoneNode::NoneNode(const CanvasType& type) {
  SetText("None");
  SetType(type);
}

unique_ptr<CanvasNode> NoneNode::Convert(const CanvasType& type) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetType(type);
  return std::move(clone);
}

void NoneNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> NoneNode::Clone() const {
  return make_unique<NoneNode>(*this);
}
