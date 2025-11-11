#include "Spire/Canvas/StandardNodes/NotNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

NotNode::NotNode() {
  DefineFunction("Not", { "value" }, MakeSignatures<NotNodeSignatures>());
}

void NotNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> NotNode::Clone() const {
  return make_unique<NotNode>(*this);
}
