#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace std;

unique_ptr<SingleOrderTaskNode> Spire::GetAskOrderTaskNode() {
  SingleOrderTaskNode baseNode("Ask Order");
  CanvasNodeBuilder builder(baseNode);
  builder.Replace(*baseNode.FindChild("side"),
    make_unique<SideNode>(Side::ASK));
  builder.SetReadOnly(*baseNode.FindChild("side"), true);
  builder.SetVisible(*baseNode.FindChild("side"), false);
  auto askOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return askOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetBidOrderTaskNode() {
  SingleOrderTaskNode baseNode("Bid Order");
  CanvasNodeBuilder builder(baseNode);
  builder.Replace(*baseNode.FindChild("side"),
    make_unique<SideNode>(Side::BID));
  builder.SetReadOnly(*baseNode.FindChild("side"), true);
  builder.SetVisible(*baseNode.FindChild("side"), false);
  auto bidOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return bidOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetLimitOrderTaskNode() {
  SingleOrderTaskNode baseNode("Limit Order");
  CanvasNodeBuilder builder(baseNode);
  builder.Replace(*baseNode.FindChild("type"),
    make_unique<OrderTypeNode>(OrderType::LIMIT));
  builder.SetReadOnly(*baseNode.FindChild("type"), true);
  builder.SetVisible(*baseNode.FindChild("type"), false);
  auto limitOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return limitOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetLimitAskOrderTaskNode() {
  auto baseNode = GetLimitOrderTaskNode();
  baseNode = baseNode->Rename("Limit Ask Order");
  CanvasNodeBuilder builder(*baseNode);
  builder.Replace(*baseNode->FindChild("side"),
    make_unique<SideNode>(Side::ASK));
  builder.SetReadOnly(*baseNode->FindChild("side"), true);
  builder.SetVisible(*baseNode->FindChild("side"), false);
  auto limitAskOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return limitAskOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetLimitBidOrderTaskNode() {
  auto baseNode = GetLimitOrderTaskNode();
  baseNode = baseNode->Rename("Limit Bid Order");
  CanvasNodeBuilder builder(*baseNode);
  builder.Replace(*baseNode->FindChild("side"),
    make_unique<SideNode>(Side::BID));
  builder.SetReadOnly(*baseNode->FindChild("side"), true);
  builder.SetVisible(*baseNode->FindChild("side"), false);
  auto limitBidOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return limitBidOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetMarketOrderTaskNode() {
  SingleOrderTaskNode baseNode("Market Order");
  CanvasNodeBuilder builder(baseNode);
  builder.Replace(*baseNode.FindChild("type"),
    make_unique<OrderTypeNode>(OrderType::MARKET));
  builder.SetReadOnly(*baseNode.FindChild("type"), true);
  builder.SetVisible(*baseNode.FindChild("type"), false);
  builder.SetReadOnly(*baseNode.FindChild("price"), true);
  builder.SetVisible(*baseNode.FindChild("price"), false);
  auto marketOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return marketOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetMarketAskOrderTaskNode() {
  auto baseNode = GetMarketOrderTaskNode();
  baseNode = baseNode->Rename("Market Ask Order");
  CanvasNodeBuilder builder(*baseNode);
  builder.Replace(*baseNode->FindChild("side"),
    make_unique<SideNode>(Side::ASK));
  builder.SetReadOnly(*baseNode->FindChild("side"), true);
  builder.SetVisible(*baseNode->FindChild("side"), false);
  auto marketAskOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return marketAskOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetMarketBidOrderTaskNode() {
  auto baseNode = GetMarketOrderTaskNode();
  baseNode = baseNode->Rename("Market Bid Order");
  CanvasNodeBuilder builder(*baseNode);
  builder.Replace(*baseNode->FindChild("side"),
    make_unique<SideNode>(Side::BID));
  builder.SetReadOnly(*baseNode->FindChild("side"), true);
  builder.SetVisible(*baseNode->FindChild("side"), false);
  auto marketBidOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return marketBidOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetPeggedOrderTaskNode(
    bool isPriceOptional) {
  SingleOrderTaskNode baseNode("Pegged Order");
  CanvasNodeBuilder builder(baseNode);
  builder.Replace(*baseNode.FindChild("type"),
    make_unique<OrderTypeNode>(OrderType::PEGGED));
  builder.SetReadOnly(*baseNode.FindChild("type"), true);
  builder.SetVisible(*baseNode.FindChild("type"), false);
  if(isPriceOptional) {
    builder.Replace(*baseNode.FindChild("price"),
      LinkedNode::SetReferent(OptionalPriceNode(), "security"));
  }
  auto peggedOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return peggedOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetPeggedAskOrderTaskNode(
    bool isPriceOptional) {
  auto baseNode = GetPeggedOrderTaskNode(isPriceOptional);
  baseNode = baseNode->Rename("Pegged Ask Order");
  CanvasNodeBuilder builder(*baseNode);
  builder.Replace(*baseNode->FindChild("side"),
    make_unique<SideNode>(Side::ASK));
  builder.SetReadOnly(*baseNode->FindChild("side"), true);
  builder.SetVisible(*baseNode->FindChild("side"), false);
  auto peggedAskOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return peggedAskOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::GetPeggedBidOrderTaskNode(
    bool isPriceOptional) {
  auto baseNode = GetPeggedOrderTaskNode(isPriceOptional);
  baseNode = baseNode->Rename("Pegged Bid Order");
  CanvasNodeBuilder builder(*baseNode);
  builder.Replace(*baseNode->FindChild("side"),
    make_unique<SideNode>(Side::BID));
  builder.SetReadOnly(*baseNode->FindChild("side"), true);
  builder.SetVisible(*baseNode->FindChild("side"), false);
  auto peggedBidOrderTaskNode =
    StaticCast<std::unique_ptr<SingleOrderTaskNode>>(builder.Make());
  return peggedBidOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::MakeOrderTaskNodeFromOrderFields(
    const OrderFields& orderFields, const UserProfile& userProfile) {
  auto node = SingleOrderTaskNode().Replace(
    SingleOrderTaskNode::SECURITY_PROPERTY, make_unique<SecurityNode>(
      orderFields.m_security, userProfile.GetMarketDatabase()))->Replace(
      SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
      make_unique<OrderTypeNode>(orderFields.m_type))->Replace(
      SingleOrderTaskNode::SIDE_PROPERTY,
      make_unique<SideNode>(orderFields.m_side))->Replace(
      SingleOrderTaskNode::DESTINATION_PROPERTY,
      make_unique<DestinationNode>(orderFields.m_destination))->Replace(
      SingleOrderTaskNode::QUANTITY_PROPERTY,
      make_unique<IntegerNode>(orderFields.m_quantity))->Replace(
      SingleOrderTaskNode::PRICE_PROPERTY,
      make_unique<MoneyNode>(orderFields.m_price))->Replace(
      SingleOrderTaskNode::CURRENCY_PROPERTY,
      make_unique<CurrencyNode>(orderFields.m_currency,
      userProfile.GetCurrencyDatabase().FromId(
      orderFields.m_currency).m_code.GetData()))->Replace(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      make_unique<TimeInForceNode>(orderFields.m_timeInForce));
  return StaticCast<std::unique_ptr<SingleOrderTaskNode>>(std::move(node));
}
