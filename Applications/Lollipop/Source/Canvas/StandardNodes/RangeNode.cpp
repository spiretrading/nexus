#include "Spire/Canvas/StandardNodes/RangeNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

RangeNode::RangeNode() {
  DefineFunction("Range", { "lower", "upper" },
    MakeSignatures<RangeNodeSignatures>());
}

void RangeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> RangeNode::Clone() const {
  return make_unique<RangeNode>(*this);
}
