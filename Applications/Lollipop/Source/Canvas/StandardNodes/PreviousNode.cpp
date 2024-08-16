#include "Spire/Canvas/StandardNodes/PreviousNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;

PreviousNode::PreviousNode() {
  SetText("Previous");
  SetType(UnionType::GetAnyValueType());
  AddChild("source", make_unique<NoneNode>(UnionType::GetAnyValueType()));
}

std::unique_ptr<CanvasNode> PreviousNode::Replace(
    const CanvasNode& child, std::unique_ptr<CanvasNode> replacement) const {
  auto clone = StaticCast<std::unique_ptr<PreviousNode>>(
    SignatureNode::Replace(child, std::move(replacement)));
  clone->SetType(clone->GetChildren().front().GetType());
  return clone;
}

void PreviousNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> PreviousNode::Clone() const {
  return std::make_unique<PreviousNode>(*this);
}

const std::vector<PreviousNode::Signature>& PreviousNode::GetSignatures() const {
  static auto signatures = [] {
    auto signatures = std::vector<PreviousNode::Signature>();
    for(auto& valueType : UnionType::GetAnyValueType().GetCompatibleTypes()) {
      auto signature = PreviousNode::Signature();
      signature.push_back(valueType);
      signature.push_back(valueType);
      signatures.emplace_back(std::move(signature));
    }
    return signatures;
  }();
  return signatures;
}

PreviousNode::PreviousNode(ReceiveBuilder) {}
