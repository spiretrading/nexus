#include "Spire/Canvas/StandardNodes/LastNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

LastNode::LastNode() {
  DefineFunction("Last", { "expression" },
    MakeSignatures<LastNodeSignatures>());
}

void LastNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> LastNode::Clone() const {
  return make_unique<LastNode>(*this);
}

LastNode::LastNode(ReceiveBuilder) {}
