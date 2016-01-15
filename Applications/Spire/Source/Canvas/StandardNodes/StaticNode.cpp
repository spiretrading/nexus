#include "Spire/Canvas/StandardNodes/StaticNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

StaticNode::StaticNode() {
  DefineFunction("Static", { "expression" },
    BuildSignatures<StaticNodeSignatures>());
}

void StaticNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> StaticNode::Clone() const {
  return make_unique<StaticNode>(*this);
}

StaticNode::StaticNode(ReceiveBuilder) {}
