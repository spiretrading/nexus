#include "Spire/Canvas/ControlNodes/UntilNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/TaskType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

UntilNode::UntilNode() {
  AddChild("condition", BuildDefaultCanvasNode(BooleanType::GetInstance()));
  AddChild("task", BuildDefaultCanvasNode(TaskType::GetInstance()));
  SetText("Until");
  SetType(TaskType::GetInstance());
}

void UntilNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> UntilNode::Clone() const {
  return make_unique<UntilNode>(*this);
}

UntilNode::UntilNode(ReceiveBuilder) {}
