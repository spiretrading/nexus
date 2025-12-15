#include "Spire/Canvas/StandardNodes/DistinctNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Spire;

DistinctNode::DistinctNode() {
  SetText("Distinct");
  SetType(UnionType::GetAnyValueType());
  AddChild("source", make_unique<NoneNode>(UnionType::GetAnyValueType()));
}

std::unique_ptr<CanvasNode> DistinctNode::Replace(
    const CanvasNode& child, std::unique_ptr<CanvasNode> replacement) const {
  auto clone = static_pointer_cast<DistinctNode>(
    SignatureNode::Replace(child, std::move(replacement)));
  clone->SetType(clone->GetChildren().front().GetType());
  return clone;
}

void DistinctNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> DistinctNode::Clone() const {
  return std::make_unique<DistinctNode>(*this);
}

const std::vector<DistinctNode::Signature>& DistinctNode::GetSignatures() const {
  static auto signatures = [] {
    auto signatures = std::vector<DistinctNode::Signature>();
    for(auto& valueType : UnionType::GetAnyValueType().GetCompatibleTypes()) {
      auto signature = DistinctNode::Signature();
      signature.push_back(valueType);
      signature.push_back(valueType);
      signatures.emplace_back(std::move(signature));
    }
    return signatures;
  }();
  return signatures;
}
