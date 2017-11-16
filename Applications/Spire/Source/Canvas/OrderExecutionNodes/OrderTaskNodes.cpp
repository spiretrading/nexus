#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/Tasks/SingleOrderTask.hpp"
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
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tasks;
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
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
    UniqueStaticCast<SingleOrderTaskNode>(builder.Build());
  return peggedBidOrderTaskNode;
}

unique_ptr<SingleOrderTaskNode> Spire::BuildOrderTaskNodeFromOrderFields(
    const OrderFields& orderFields, const UserProfile& userProfile) {
  auto node = SingleOrderTaskNode().Replace(
    BaseSingleOrderTaskFactory::SECURITY, make_unique<SecurityNode>(
      orderFields.m_security, userProfile.GetMarketDatabase()))->Replace(
      BaseSingleOrderTaskFactory::ORDER_TYPE,
      make_unique<OrderTypeNode>(orderFields.m_type))->Replace(
      BaseSingleOrderTaskFactory::SIDE,
      make_unique<SideNode>(orderFields.m_side))->Replace(
      BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(orderFields.m_destination))->Replace(
      BaseSingleOrderTaskFactory::QUANTITY,
      make_unique<IntegerNode>(orderFields.m_quantity))->Replace(
      BaseSingleOrderTaskFactory::PRICE,
      make_unique<MoneyNode>(orderFields.m_price))->Replace(
      BaseSingleOrderTaskFactory::CURRENCY,
      make_unique<CurrencyNode>(orderFields.m_currency,
      userProfile.GetCurrencyDatabase().FromId(
      orderFields.m_currency).m_code.GetData()))->Replace(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(orderFields.m_timeInForce));
  return UniqueStaticCast<SingleOrderTaskNode>(std::move(node));
}
