#include "Spire/Canvas/StandardNodes/GreaterOrEqualsNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/ComparisonSignatures.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

GreaterOrEqualsNode::GreaterOrEqualsNode() {
  DefineFunction("Greater/Equals", { "left", "right" },
    MakeSignatures<ComparisonSignatures>());
}

void GreaterOrEqualsNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> GreaterOrEqualsNode::Clone() const {
  return make_unique<GreaterOrEqualsNode>(*this);
}

GreaterOrEqualsNode::GreaterOrEqualsNode(ReceiveBuilder) {}
