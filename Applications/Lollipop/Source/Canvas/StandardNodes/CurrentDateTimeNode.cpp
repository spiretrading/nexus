#include "Spire/Canvas/StandardNodes/CurrentDateTimeNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

CurrentDateTimeNode::CurrentDateTimeNode() {
  SetText("Current Date/Time");
  SetType(DateTimeType::GetInstance());
}

void CurrentDateTimeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> CurrentDateTimeNode::Clone() const {
  return make_unique<CurrentDateTimeNode>(*this);
}
