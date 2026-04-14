#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

InteractionsNode::InteractionsNode()
  : InteractionsNode(Ticker(), InteractionsKeyBindingsModel()) {}

InteractionsNode::InteractionsNode(
    Ticker ticker, const InteractionsKeyBindingsModel& interactions) {
  SetText("");
  SetType(IntegerType::GetInstance());
  auto tickerNode = std::unique_ptr<CanvasNode>(
    std::make_unique<TickerNode>(std::move(ticker)));
  tickerNode = tickerNode->SetVisible(false);
  AddChild("ticker", std::move(tickerNode));
  auto defaultQuantityNode = LinkedNode::SetReferent(
    IntegerNode(interactions.get_default_quantity()->get()), "ticker");
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

void Spire::apply(const InteractionsNode& node, KeyBindingsModel& keyBindings) {
  if(auto tickerNode = node.FindChild("ticker")) {
    if(auto ticker =
        dynamic_cast<const TickerNode*>(&*tickerNode)) {
      apply(
        node, *keyBindings.get_interactions_key_bindings(ticker->GetValue()));
    }
  }
}
