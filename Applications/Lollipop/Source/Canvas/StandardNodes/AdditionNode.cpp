#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;

AdditionNode::AdditionNode() {
  DefineFunction("Add", { "left", "right" },
    MakeSignatures<AdditionNodeSignatures>());
}

void AdditionNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> AdditionNode::Clone() const {
  return std::make_unique<AdditionNode>(*this);
}
