#include "Spire/Canvas/ControlNodes/WhenNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;

WhenNode::WhenNode() {
  DefineFunction("When", { "condition", "series" },
    MakeSignatures<Signatures>());
}

void WhenNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> WhenNode::Clone() const {
  return std::make_unique<WhenNode>(*this);
}
