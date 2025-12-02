#include "Spire/Canvas/StandardNodes/FilterNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"
#include "Spire/Canvas/ValueNodes/BooleanNode.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  vector<FilterNode::Signature> MakeFilterSignatures() {
    vector<FilterNode::Signature> signatures;
    for(const auto& valueType :
        UnionType::GetAnyValueType().GetCompatibleTypes()) {
      FilterNode::Signature signature;
      signature.push_back(valueType);
      signature.push_back(BooleanType::GetInstance());
      signature.push_back(valueType);
      signatures.emplace_back(std::move(signature));
    }
    return signatures;
  }
}

FilterNode::FilterNode() {
  SetText("Filter");
  SetType(UnionType::GetAnyValueType());
  AddChild("expression", make_unique<NoneNode>(UnionType::GetAnyValueType()));
  AddChild("condition", make_unique<BooleanNode>(true));
}

unique_ptr<CanvasNode> FilterNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  auto clone = static_pointer_cast<FilterNode>(
    SignatureNode::Replace(child, std::move(replacement)));
  clone->SetType(clone->GetChildren().front().GetType());
  return std::move(clone);
}

void FilterNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> FilterNode::Clone() const {
  return make_unique<FilterNode>(*this);
}

const vector<FilterNode::Signature>& FilterNode::GetSignatures() const {
  static vector<FilterNode::Signature> signatures = MakeFilterSignatures();
  return signatures;
}
