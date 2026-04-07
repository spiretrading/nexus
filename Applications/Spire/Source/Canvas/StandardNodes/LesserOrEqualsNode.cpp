#include "Spire/Canvas/StandardNodes/LesserOrEqualsNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/ComparisonSignatures.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

LesserOrEqualsNode::LesserOrEqualsNode() {
  DefineFunction("Lesser/Equals", { "left", "right" },
    MakeSignatures<ComparisonSignatures>());
}

void LesserOrEqualsNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> LesserOrEqualsNode::Clone() const {
  return make_unique<LesserOrEqualsNode>(*this);
}
