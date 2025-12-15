#include "Spire/Canvas/StandardNodes/FirstNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

FirstNode::FirstNode() {
  DefineFunction("First", { "expression" },
    MakeSignatures<FirstNodeSignatures>());
}

void FirstNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> FirstNode::Clone() const {
  return make_unique<FirstNode>(*this);
}
