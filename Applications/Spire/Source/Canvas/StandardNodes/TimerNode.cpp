#include "Spire/Canvas/StandardNodes/TimerNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

TimerNode::TimerNode() {
  SetText("Timer");
  AddChild("period", MakeDefaultCanvasNode(DurationType::GetInstance()));
  SetType(IntegerType::GetInstance());
}

void TimerNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TimerNode::Clone() const {
  return make_unique<TimerNode>(*this);
}
