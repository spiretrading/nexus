#include "Spire/Canvas/StandardNodes/AbsNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

AbsNode::AbsNode() {
  DefineFunction("Abs", { "value" }, MakeSignatures<AbsNodeSignatures>());
}

void AbsNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> AbsNode::Clone() const {
  return make_unique<AbsNode>(*this);
}

AbsNode::AbsNode(ReceiveBuilder) {}
