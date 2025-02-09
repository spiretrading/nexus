#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;

BlotterTaskMonitorNode::BlotterTaskMonitorNode() {
  SetText("Task Monitor");
  SetType(UnionType::GetAnyType());
  AddChild("monitor", make_unique<NoneNode>());
}

std::unique_ptr<CanvasNode> BlotterTaskMonitorNode::Replace(
    const CanvasNode& child, std::unique_ptr<CanvasNode> replacement) const {
  auto clone = CanvasNode::Clone(*this);
  auto type = std::shared_ptr<CanvasType>(replacement->GetType());
  clone->SetChild(clone->GetChildren().front(), std::move(replacement));
  clone->SetType(*type);
  return clone;
}

void BlotterTaskMonitorNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> BlotterTaskMonitorNode::Clone() const {
  return make_unique<BlotterTaskMonitorNode>(*this);
}

BlotterTaskMonitorNode::BlotterTaskMonitorNode(ReceiveBuilder) {}
