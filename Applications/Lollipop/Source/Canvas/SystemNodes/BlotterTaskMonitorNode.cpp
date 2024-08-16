#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

BlotterTaskMonitorNode::BlotterTaskMonitorNode() {
  SetText("Task Monitor");
  SetType(UnionType::GetAnyType());
  AddChild("monitor", make_unique<NoneNode>());
}

unique_ptr<CanvasNode> BlotterTaskMonitorNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  auto clone = CanvasNode::Clone(*this);
  std::shared_ptr<CanvasType> type = replacement->GetType();
  clone->SetChild(clone->GetChildren().front(), std::move(replacement));
  clone->SetType(*type);
  return std::move(clone);
}

void BlotterTaskMonitorNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> BlotterTaskMonitorNode::Clone() const {
  return make_unique<BlotterTaskMonitorNode>(*this);
}

BlotterTaskMonitorNode::BlotterTaskMonitorNode(ReceiveBuilder) {}
