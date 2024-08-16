#include "Spire/Canvas/StandardNodes/LesserNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/ComparisonSignatures.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

LesserNode::LesserNode() {
  DefineFunction("Lesser", { "left", "right" },
    MakeSignatures<ComparisonSignatures>());
}

void LesserNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> LesserNode::Clone() const {
  return make_unique<LesserNode>(*this);
}

LesserNode::LesserNode(ReceiveBuilder) {}
