#include "Spire/Canvas/StandardNodes/CurrentDateNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

CurrentDateNode::CurrentDateNode() {
  SetText("Current Date");
  SetType(DateTimeType::GetInstance());
}

void CurrentDateNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> CurrentDateNode::Clone() const {
  return make_unique<CurrentDateNode>(*this);
}

CurrentDateNode::CurrentDateNode(ReceiveBuilder) {}
