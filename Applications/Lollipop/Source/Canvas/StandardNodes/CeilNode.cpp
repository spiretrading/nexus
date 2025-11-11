#include "Spire/Canvas/StandardNodes/CeilNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/MathSignatures.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

CeilNode::CeilNode() {
  DefineFunction("Ceil", { "value", "to_closest" },
    MakeSignatures<RoundingNodeSignatures>());
}

void CeilNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> CeilNode::Clone() const {
  return make_unique<CeilNode>(*this);
}
