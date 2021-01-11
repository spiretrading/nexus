#include "Spire/Canvas/StandardNodes/SubtractionNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

SubtractionNode::SubtractionNode() {
  DefineFunction("Subtract", { "left", "right" },
    MakeSignatures<SubtractionNodeSignatures>());
}

void SubtractionNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> SubtractionNode::Clone() const {
  return make_unique<SubtractionNode>(*this);
}

SubtractionNode::SubtractionNode(ReceiveBuilder) {}
