#include "Spire/Canvas/StandardNodes/FoldOperandNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

FoldOperandNode::FoldOperandNode(Side side)
    : m_side(side) {
  SetType(UnionType::GetAnyValueType());
  SetText("Fold Operand");
}

unique_ptr<CanvasNode> FoldOperandNode::Convert(const CanvasType& type) const {
  if(type.GetCompatibility(OrderReferenceType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
  }
  auto clone = CanvasNode::Clone(*this);
  clone->SetType(type);
  return std::move(clone);
}

void FoldOperandNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> FoldOperandNode::Clone() const {
  return make_unique<FoldOperandNode>(*this);
}

FoldOperandNode::FoldOperandNode(ReceiveBuilder) {}
