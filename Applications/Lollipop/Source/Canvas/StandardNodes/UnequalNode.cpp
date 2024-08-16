#include "Spire/Canvas/StandardNodes/UnequalNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/ComparisonSignatures.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

UnequalNode::UnequalNode() {
  DefineFunction("Unequal", { "left", "right" },
    MakeSignatures<ComparisonSignatures>());
}

void UnequalNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> UnequalNode::Clone() const {
  return make_unique<UnequalNode>(*this);
}

UnequalNode::UnequalNode(ReceiveBuilder) {}
