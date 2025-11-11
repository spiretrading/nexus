#include "Spire/Canvas/StandardNodes/FloorNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/MathSignatures.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

FloorNode::FloorNode() {
  DefineFunction("Floor", { "value", "to_closest" },
    MakeSignatures<RoundingNodeSignatures>());
}

void FloorNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> FloorNode::Clone() const {
  return make_unique<FloorNode>(*this);
}
