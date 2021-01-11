#include "Spire/Canvas/ControlNodes/UntilNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;

UntilNode::UntilNode() {
  DefineFunction("Until", { "condition", "series" },
    MakeSignatures<Signatures>());
}

void UntilNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> UntilNode::Clone() const {
  return std::make_unique<UntilNode>(*this);
}

UntilNode::UntilNode(ReceiveBuilder) {}
