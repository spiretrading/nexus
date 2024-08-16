#include "Spire/Canvas/StandardNodes/EqualsNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/ComparisonSignatures.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

EqualsNode::EqualsNode() {
  DefineFunction("Equals", { "left", "right" },
    MakeSignatures<EqualitySignatures>());
}

void EqualsNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> EqualsNode::Clone() const {
  return make_unique<EqualsNode>(*this);
}

EqualsNode::EqualsNode(ReceiveBuilder) {}
