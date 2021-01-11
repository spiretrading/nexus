#include "Spire/Canvas/StandardNodes/GreaterNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/ComparisonSignatures.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

GreaterNode::GreaterNode() {
  DefineFunction("Greater", { "left", "right" },
    MakeSignatures<ComparisonSignatures>());
}

void GreaterNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> GreaterNode::Clone() const {
  return make_unique<GreaterNode>(*this);
}

GreaterNode::GreaterNode(ReceiveBuilder) {}
