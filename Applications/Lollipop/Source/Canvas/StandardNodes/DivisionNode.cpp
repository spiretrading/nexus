#include "Spire/Canvas/StandardNodes/DivisionNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

DivisionNode::DivisionNode() {
  DefineFunction("Divide", { "numerator", "denominator" },
    MakeSignatures<DivisionNodeSignatures>());
}

void DivisionNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> DivisionNode::Clone() const {
  return make_unique<DivisionNode>(*this);
}

DivisionNode::DivisionNode(ReceiveBuilder) {}
