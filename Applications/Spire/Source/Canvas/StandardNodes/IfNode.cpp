#include "Spire/Canvas/StandardNodes/IfNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

IfNode::IfNode() {
  DefineFunction("If", { "condition", "consequent", "default" },
    MakeSignatures<IfNodeSignatures>());
}

void IfNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> IfNode::Clone() const {
  return make_unique<IfNode>(*this);
}

IfNode::IfNode(ReceiveBuilder) {}
