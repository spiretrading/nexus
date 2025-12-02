#include "Spire/Canvas/StandardNodes/MaxNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/MathSignatures.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

MaxNode::MaxNode() {
  DefineFunction("Max", { "a", "b" }, MakeSignatures<ExtremaNodeSignatures>());
}

void MaxNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> MaxNode::Clone() const {
  return make_unique<MaxNode>(*this);
}
