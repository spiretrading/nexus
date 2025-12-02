#include "Spire/Canvas/StandardNodes/CountNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"

using namespace Beam;
using namespace Spire;

CountNode::CountNode() {
  SetText("Count");
  SetType(IntegerType::GetInstance());
  AddChild("source", std::make_unique<NoneNode>(UnionType::GetAnyValueType()));
}

void CountNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> CountNode::Clone() const {
  return std::make_unique<CountNode>(*this);
}

const std::vector<CountNode::Signature>& CountNode::GetSignatures() const {
  static auto signatures = [] {
    auto signatures = std::vector<CountNode::Signature>();
    for(auto& valueType : UnionType::GetAnyValueType().GetCompatibleTypes()) {
      auto signature = CountNode::Signature();
      signature.push_back(valueType);
      signature.push_back(IntegerType::GetInstance());
      signatures.emplace_back(std::move(signature));
    }
    return signatures;
  }();
  return signatures;
}
