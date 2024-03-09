#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

InteractionsNode::InteractionsNode()
  : InteractionsNode(
      Security(), GetDefaultMarketDatabase(), InteractionsKeyBindingsModel()) {}

InteractionsNode::InteractionsNode(Security security,
    const MarketDatabase& marketDatabase,
    const InteractionsKeyBindingsModel& interactions) {
  SetText("");
  SetType(IntegerType::GetInstance());
  auto securityNode = std::unique_ptr<CanvasNode>(
    std::make_unique<SecurityNode>(std::move(security), marketDatabase));
  securityNode = securityNode->SetVisible(false);
  AddChild("security", std::move(securityNode));
  auto defaultQuantityNode = LinkedNode::SetReferent(
    IntegerNode(interactions.get_default_quantity()->get()), "security");
  AddChild("default_quantity", std::move(defaultQuantityNode));
  AddChild("quantity_increment", std::make_unique<IntegerNode>(
    interactions.get_quantity_increment(Qt::NoModifier)->get()));
  AddChild("price_increment", std::make_unique<MoneyNode>(
    interactions.get_price_increment(Qt::NoModifier)->get()));
}

void InteractionsNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> InteractionsNode::Clone() const {
  return std::make_unique<InteractionsNode>(*this);
}

void Spire::apply(const InteractionsNode& node,
    InteractionsKeyBindingsModel& interactions) {
  if(auto defaultQuantityNode = node.FindChild("default_quantity")) {
    if(auto integerNode =
        dynamic_cast<const IntegerNode*>(&*defaultQuantityNode)) {
      interactions.get_default_quantity()->set(integerNode->GetValue());
    }
  }
  if(auto quantityIncrementNode = node.FindChild("quantity_increment")) {
    if(auto quantityNode =
        dynamic_cast<const IntegerNode*>(&*quantityIncrementNode)) {
      interactions.get_quantity_increment(Qt::NoModifier)->set(
        quantityNode->GetValue());
    }
  }
  if(auto priceIncrementNode = node.FindChild("price_increment")) {
    if(auto priceNode = dynamic_cast<const MoneyNode*>(&*priceIncrementNode)) {
      interactions.get_price_increment(Qt::NoModifier)->set(
        priceNode->GetValue());
    }
  }
}
