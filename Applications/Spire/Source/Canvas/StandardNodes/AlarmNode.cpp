#include "Spire/Canvas/StandardNodes/AlarmNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

AlarmNode::AlarmNode() {
  SetText("Alarm");
  AddChild("time", MakeDefaultCanvasNode(DateTimeType::GetInstance()));
  SetType(BooleanType::GetInstance());
}

void AlarmNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> AlarmNode::Clone() const {
  return make_unique<AlarmNode>(*this);
}

AlarmNode::AlarmNode(ReceiveBuilder) {}
