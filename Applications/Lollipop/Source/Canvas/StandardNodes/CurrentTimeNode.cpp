#include "Spire/Canvas/StandardNodes/CurrentTimeNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

CurrentTimeNode::CurrentTimeNode() {
  SetText("Current Time");
  SetType(DurationType::GetInstance());
}

void CurrentTimeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> CurrentTimeNode::Clone() const {
  return make_unique<CurrentTimeNode>(*this);
}
