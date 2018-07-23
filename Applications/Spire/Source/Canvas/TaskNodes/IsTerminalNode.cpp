#include "Spire/Canvas/TaskNodes/IsTerminalNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/TaskStateType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

IsTerminalNode::IsTerminalNode() {
  SetText("Is Terminal");
  AddChild("state", BuildDefaultCanvasNode(TaskStateType::GetInstance()));
  SetType(BooleanType::GetInstance());
}

void IsTerminalNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> IsTerminalNode::Clone() const {
  return make_unique<IsTerminalNode>(*this);
}

IsTerminalNode::IsTerminalNode(ReceiveBuilder) {}
