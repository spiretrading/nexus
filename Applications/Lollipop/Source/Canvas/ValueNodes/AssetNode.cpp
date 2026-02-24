#include "Spire/Canvas/ValueNodes/AssetNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

AssetNode::AssetNode() {
  SetText("None");
}

AssetNode::AssetNode(Asset value)
    : ValueNode<AssetType>(value) {
  SetText("Asset");
}

std::unique_ptr<AssetNode> AssetNode::SetValue(Asset value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText("Asset");
  return clone;
}

void AssetNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> AssetNode::Clone() const {
  return std::make_unique<AssetNode>(*this);
}

std::unique_ptr<CanvasNode> AssetNode::Reset() const {
  return std::make_unique<AssetNode>();
}
