#include "Spire/Canvas/TaskNodes/TaskStateMonitorNode.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/TaskStateType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

TaskStateMonitorNode::TaskStateMonitorNode() {
  SetText("Task State Monitor");
  SetType(TaskStateType::GetInstance());
  AddChild("source", make_unique<ReferenceNode>());
}

unique_ptr<CanvasNode> TaskStateMonitorNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  if(&child == &GetChildren().front()) {
    if(dynamic_cast<const ReferenceNode*>(replacement.get())) {
      return CanvasNode::Replace(child, std::move(replacement));
    }
    BOOST_THROW_EXCEPTION(CanvasOperationException("Must reference a task."));
  }
  return CanvasNode::Replace(child, std::move(replacement));
}

void TaskStateMonitorNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TaskStateMonitorNode::Clone() const {
  return make_unique<TaskStateMonitorNode>(*this);
}

TaskStateMonitorNode::TaskStateMonitorNode(ReceiveBuilder) {}
