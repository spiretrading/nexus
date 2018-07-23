#include "Spire/Canvas/ValueNodes/TaskStateNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace Nexus;
using namespace Spire;
using namespace std;

TaskStateNode::TaskStateNode()
    : ValueNode(Task::State::INITIALIZING) {
  SetText(ToString(GetValue()));
}

TaskStateNode::TaskStateNode(Task::State value)
    : ValueNode(value) {
  SetText(ToString(GetValue()));
}

unique_ptr<TaskStateNode> TaskStateNode::SetValue(Task::State value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(ToString(clone->GetValue()));
  return clone;
}

void TaskStateNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TaskStateNode::Clone() const {
  return make_unique<TaskStateNode>(*this);
}

unique_ptr<CanvasNode> TaskStateNode::Reset() const {
  return make_unique<TaskStateNode>();
}
