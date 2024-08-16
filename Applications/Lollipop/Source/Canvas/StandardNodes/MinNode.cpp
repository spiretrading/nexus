#include "Spire/Canvas/StandardNodes/MinNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/MathSignatures.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

MinNode::MinNode() {
  DefineFunction("Min", { "a", "b" }, MakeSignatures<ExtremaNodeSignatures>());
}

void MinNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> MinNode::Clone() const {
  return make_unique<MinNode>(*this);
}

MinNode::MinNode(ReceiveBuilder) {}
