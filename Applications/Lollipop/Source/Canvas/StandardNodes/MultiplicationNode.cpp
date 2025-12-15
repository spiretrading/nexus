#include "Spire/Canvas/StandardNodes/MultiplicationNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

MultiplicationNode::MultiplicationNode() {
  DefineFunction("Multiply", { "left", "right" },
    MakeSignatures<MultiplicationNodeSignatures>());
}

void MultiplicationNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> MultiplicationNode::Clone() const {
  return make_unique<MultiplicationNode>(*this);
}
