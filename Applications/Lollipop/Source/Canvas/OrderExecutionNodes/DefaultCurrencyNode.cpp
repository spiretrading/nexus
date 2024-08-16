#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

DefaultCurrencyNode::DefaultCurrencyNode() {
  SetText("Default Currency");
  SetType(CurrencyType::GetInstance());
  AddChild("source", make_unique<SecurityNode>());
}

void DefaultCurrencyNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> DefaultCurrencyNode::Clone() const {
  return make_unique<DefaultCurrencyNode>(*this);
}

DefaultCurrencyNode::DefaultCurrencyNode(ReceiveBuilder) {}
