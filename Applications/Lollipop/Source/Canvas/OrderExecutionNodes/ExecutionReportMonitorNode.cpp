#include "Spire/Canvas/OrderExecutionNodes/ExecutionReportMonitorNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/ExecutionReportRecordType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

ExecutionReportMonitorNode::ExecutionReportMonitorNode() {
  SetText("Execution Report Monitor");
  SetType(GetExecutionReportRecordType());
  AddChild("source", make_unique<ReferenceNode>("",
    OrderReferenceType::GetInstance()));
}

unique_ptr<CanvasNode> ExecutionReportMonitorNode::Replace(
    const CanvasNode& child, unique_ptr<CanvasNode> replacement) const {
  if(&child == &GetChildren().front()) {
    if(dynamic_cast<const ReferenceNode*>(replacement.get())) {
      return CanvasNode::Replace(child, std::move(replacement));
    }
    BOOST_THROW_EXCEPTION(CanvasOperationException("Must reference a task."));
  }
  return CanvasNode::Replace(child, std::move(replacement));
}

void ExecutionReportMonitorNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> ExecutionReportMonitorNode::Clone() const {
  return make_unique<ExecutionReportMonitorNode>(*this);
}

ExecutionReportMonitorNode::ExecutionReportMonitorNode(ReceiveBuilder) {}
