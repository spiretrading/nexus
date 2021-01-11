#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

AdditionNode::AdditionNode() {
  DefineFunction("Add", { "left", "right" },
    MakeSignatures<AdditionNodeSignatures>());
}

void AdditionNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> AdditionNode::Clone() const {
  return make_unique<AdditionNode>(*this);
}

AdditionNode::AdditionNode(ReceiveBuilder) {}
