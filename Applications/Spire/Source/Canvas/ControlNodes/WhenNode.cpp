#include "Spire/Canvas/ControlNodes/WhenNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

WhenNode::WhenNode() {
  AddChild("condition", BuildDefaultCanvasNode(BooleanType::GetInstance()));
  AddChild("task", BuildDefaultCanvasNode(OrderReferenceType::GetInstance()));
  SetText("When");
  SetType(OrderReferenceType::GetInstance());
}

void WhenNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> WhenNode::Clone() const {
  return make_unique<WhenNode>(*this);
}

WhenNode::WhenNode(ReceiveBuilder) {}
