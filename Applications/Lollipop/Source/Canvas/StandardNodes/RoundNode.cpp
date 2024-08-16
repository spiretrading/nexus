#include "Spire/Canvas/StandardNodes/RoundNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/MathSignatures.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

RoundNode::RoundNode() {
  DefineFunction("Round", { "value", "to_closest" },
    MakeSignatures<RoundingNodeSignatures>());
}

void RoundNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> RoundNode::Clone() const {
  return make_unique<RoundNode>(*this);
}

RoundNode::RoundNode(ReceiveBuilder) {}
