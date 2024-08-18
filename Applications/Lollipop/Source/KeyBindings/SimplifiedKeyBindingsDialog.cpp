#include "Spire/KeyBindings/SimplifiedKeyBindingsDialog.hpp"
#include <QKeyEvent>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_SimplifiedKeyBindingsDialog.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  const string KEY_BINDING_IDENTIFIER = "spire_default-key-binding";
  enum {
    ORDER_TYPE_COLUMN = 0,
    TASK_KEY_COLUMN,
  };

  enum {
    CANCEL_OPTION_COLUMN = 0,
    CANCEL_KEY_COLUMN,
  };

  void PopulateAsxOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "ASX TradeMatch Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "ASX TradeMatch Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "ASX TradeMatch Market Bid"));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "ASX TradeMatch Market Ask"));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename(
      "ASX TradeMatch Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename(
      "ASX TradeMatch Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ASX TradeMatch Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ASX TradeMatch Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ASX TradeMatch Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ASX TradeMatch Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ASX TradeMatch Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ASX TradeMatch Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateCxaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "CXA Limit Bid"));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "CXA Limit Ask"));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "CXA Market Bid"));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "CXA Market Ask"));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("CXA Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("CXA Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "CXA Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "CXA Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "CXA Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "CXA Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "CXA Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "CXA Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CXA()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateAlphaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder alphaLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "Alpha Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    alphaLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    alphaLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    alphaLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaLimitBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    alphaLimitBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    alphaLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaLimitBid.Make());
    CanvasNodeBuilder alphaLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "Alpha Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    alphaLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    alphaLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    alphaLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaLimitAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    alphaLimitAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    alphaLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaLimitAsk.Make());
    CanvasNodeBuilder alphaMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "Alpha Market Bid"));
    alphaMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    alphaMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    alphaMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaMarketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    alphaMarketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    alphaMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaMarketBid.Make());
    CanvasNodeBuilder alphaMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "Alpha Market Ask"));
    alphaMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    alphaMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    alphaMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaMarketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    alphaMarketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    alphaMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaMarketAsk.Make());
    CanvasNodeBuilder alphaBuy(*GetMarketBidOrderTaskNode()->Rename(
      "Alpha Buy"));
    alphaBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    alphaBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    alphaBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    alphaBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    alphaBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    alphaBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaBuy.Make());
    CanvasNodeBuilder alphaSell(*GetMarketAskOrderTaskNode()->Rename(
      "Alpha Sell"));
    alphaSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    alphaSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    alphaSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    alphaSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    alphaSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    alphaSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaSell.Make());
  }

  void PopulateChixOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder chixLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "CHI-X Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    chixLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixLimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixLimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixLimitBid.Make());
    CanvasNodeBuilder chixLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "CHI-X Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    chixLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixLimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixLimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixLimitAsk.Make());
    CanvasNodeBuilder chixMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "CHI-X Market Bid"));
    chixMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMarketBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixMarketBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMarketBid.Make());
    CanvasNodeBuilder chixMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "CHI-X Market Ask"));
    chixMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMarketAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixMarketAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMarketAsk.Make());
    CanvasNodeBuilder chixBuy(
      *GetMarketBidOrderTaskNode()->Rename("CHI-X Buy"));
    chixBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    chixBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixBuy.Make());
    CanvasNodeBuilder chixSell(*GetMarketAskOrderTaskNode()->Rename(
      "CHI-X Sell"));
    chixSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    chixSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSell.Make());
    CanvasNodeBuilder chixFeeSensitiveLimitBid(
      *GetLimitBidOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveLimitBid.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixFeeSensitiveLimitBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixFeeSensitiveLimitBid.SetVisible("ex_destination", false);
    chixFeeSensitiveLimitBid.SetReadOnly("ex_destination", true);
    chixFeeSensitiveLimitBid.Replace(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveLimitBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixFeeSensitiveLimitBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixFeeSensitiveLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveLimitBid.Make());
    CanvasNodeBuilder chixFeeSensitiveLimitAsk(
      *GetLimitAskOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveLimitAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixFeeSensitiveLimitAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixFeeSensitiveLimitAsk.SetVisible("ex_destination", false);
    chixFeeSensitiveLimitAsk.SetReadOnly("ex_destination", true);
    chixFeeSensitiveLimitAsk.Replace(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveLimitAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixFeeSensitiveLimitAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixFeeSensitiveLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveLimitAsk.Make());
    CanvasNodeBuilder chixFeeSensitiveMarketBid(
      *GetMarketBidOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Market Bid")->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveMarketBid.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixFeeSensitiveMarketBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixFeeSensitiveMarketBid.SetVisible("ex_destination", false);
    chixFeeSensitiveMarketBid.SetReadOnly("ex_destination", true);
    chixFeeSensitiveMarketBid.Replace(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveMarketBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixFeeSensitiveMarketBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixFeeSensitiveMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveMarketBid.Make());
    CanvasNodeBuilder chixFeeSensitiveMarketAsk(
      *GetMarketAskOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Market Ask")->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveMarketAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixFeeSensitiveMarketAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixFeeSensitiveMarketAsk.SetVisible("ex_destination", false);
    chixFeeSensitiveMarketAsk.SetReadOnly("ex_destination", true);
    chixFeeSensitiveMarketAsk.Replace(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveMarketAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixFeeSensitiveMarketAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixFeeSensitiveMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveMarketAsk.Make());
    CanvasNodeBuilder chixFeeSensitiveBuy(
      *GetMarketBidOrderTaskNode()->Rename("CHI-X Fee Sensitive Buy")->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveBuy.SetVisible(
      SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    chixFeeSensitiveBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixFeeSensitiveBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixFeeSensitiveBuy.SetVisible("ex_destination", false);
    chixFeeSensitiveBuy.SetReadOnly("ex_destination", true);
    chixFeeSensitiveBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixFeeSensitiveBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixFeeSensitiveBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveBuy.Make());
    CanvasNodeBuilder chixFeeSensitiveSell(*GetMarketAskOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Sell")->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveSell.SetVisible(
      SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    chixFeeSensitiveSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixFeeSensitiveSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixFeeSensitiveSell.SetVisible("ex_destination", false);
    chixFeeSensitiveSell.SetReadOnly("ex_destination", true);
    chixFeeSensitiveSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixFeeSensitiveSell.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixFeeSensitiveSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveSell.Make());
    CanvasNodeBuilder chixSmartXDarkLimitBid(
      *GetLimitBidOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>(
      "SMRTXDARKNR"))->AddField(
      "long_life", 7735, std::make_unique<TextNode>("Y")));
    chixSmartXDarkLimitBid.SetReadOnly("long_life", true);
    chixSmartXDarkLimitBid.SetVisible("long_life", false);
    chixSmartXDarkLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixSmartXDarkLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixSmartXDarkLimitBid.SetVisible("ex_destination", false);
    chixSmartXDarkLimitBid.SetReadOnly("ex_destination", true);
    chixSmartXDarkLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkLimitBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixSmartXDarkLimitBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixSmartXDarkLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkLimitBid.Make());
    CanvasNodeBuilder chixSmartXDarkLimitAsk(
      *GetLimitAskOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>(
      "SMRTXDARKNR"))->AddField(
      "long_life", 7735, std::make_unique<TextNode>("Y")));
    chixSmartXDarkLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkLimitAsk.SetReadOnly("long_life", true);
    chixSmartXDarkLimitAsk.SetVisible("long_life", false);
    chixSmartXDarkLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixSmartXDarkLimitAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixSmartXDarkLimitAsk.SetVisible("ex_destination", false);
    chixSmartXDarkLimitAsk.SetReadOnly("ex_destination", true);
    chixSmartXDarkLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkLimitAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixSmartXDarkLimitAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixSmartXDarkLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkLimitAsk.Make());
    CanvasNodeBuilder chixSmartXDarkMarketBid(
      *GetMarketBidOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Market Bid")->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkMarketBid.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixSmartXDarkMarketBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixSmartXDarkMarketBid.SetVisible("ex_destination", false);
    chixSmartXDarkMarketBid.SetReadOnly("ex_destination", true);
    chixSmartXDarkMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkMarketBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixSmartXDarkMarketBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixSmartXDarkMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkMarketBid.Make());
    CanvasNodeBuilder chixSmartXDarkMarketAsk(
      *GetMarketAskOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Market Ask")->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkMarketAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixSmartXDarkMarketAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixSmartXDarkMarketAsk.SetVisible("ex_destination", false);
    chixSmartXDarkMarketAsk.SetReadOnly("ex_destination", true);
    chixSmartXDarkMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkMarketAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixSmartXDarkMarketAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixSmartXDarkMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkMarketAsk.Make());
    CanvasNodeBuilder chixSmartXDarkBuy(
      *GetMarketBidOrderTaskNode()->Rename("CHI-X SMART X Dark Buy")->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    chixSmartXDarkBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixSmartXDarkBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixSmartXDarkBuy.SetVisible("ex_destination", false);
    chixSmartXDarkBuy.SetReadOnly("ex_destination", true);
    chixSmartXDarkBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixSmartXDarkBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixSmartXDarkBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkBuy.Make());
    CanvasNodeBuilder chixSmartXDarkSell(*GetMarketAskOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Sell")->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkSell.SetVisible(
      SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    chixSmartXDarkSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixSmartXDarkSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixSmartXDarkSell.SetVisible("ex_destination", false);
    chixSmartXDarkSell.SetReadOnly("ex_destination", true);
    chixSmartXDarkSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixSmartXDarkSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixSmartXDarkSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkSell.Make());
    CanvasNodeBuilder chixDarkAttBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "CHI-X Dark ATT Bid")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("CXD"))->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P")));
    chixDarkAttBid.SetReadOnly("exec_inst", true);
    chixDarkAttBid.SetVisible("exec_inst", false);
    chixDarkAttBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixDarkAttBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixDarkAttBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixDarkAttBid.SetVisible("ex_destination", false);
    chixDarkAttBid.SetReadOnly("ex_destination", true);
    chixDarkAttBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixDarkAttBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixDarkAttBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixDarkAttBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixDarkAttBid.Make());
    CanvasNodeBuilder chixDarkAttAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "CHI-X Dark ATT Ask")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("CXD"))->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P")));
    chixDarkAttAsk.SetReadOnly("exec_inst", true);
    chixDarkAttAsk.SetVisible("exec_inst", false);
    chixDarkAttAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixDarkAttAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixDarkAttAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixDarkAttAsk.SetVisible("ex_destination", false);
    chixDarkAttAsk.SetReadOnly("ex_destination", true);
    chixDarkAttAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixDarkAttAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixDarkAttAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixDarkAttAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixDarkAttAsk.Make());
    CanvasNodeBuilder chixDarkMpiBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "CHI-X Dark MPI Bid")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("CXD"))->AddField("exec_inst", 18,
      std::make_unique<TextNode>("x")));
    chixDarkMpiBid.SetReadOnly("exec_inst", true);
    chixDarkMpiBid.SetVisible("exec_inst", false);
    chixDarkMpiBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixDarkMpiBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixDarkMpiBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixDarkMpiBid.SetVisible("ex_destination", false);
    chixDarkMpiBid.SetReadOnly("ex_destination", true);
    chixDarkMpiBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixDarkMpiBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixDarkMpiBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixDarkMpiBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixDarkMpiBid.Make());
    CanvasNodeBuilder chixDarkMpiAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "CHI-X Dark MPI Ask")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("CXD"))->AddField("exec_inst", 18,
      std::make_unique<TextNode>("x")));
    chixDarkMpiAsk.SetReadOnly("exec_inst", true);
    chixDarkMpiAsk.SetVisible("exec_inst", false);
    chixDarkMpiAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixDarkMpiAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixDarkMpiAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixDarkMpiAsk.SetVisible("ex_destination", false);
    chixDarkMpiAsk.SetReadOnly("ex_destination", true);
    chixDarkMpiAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixDarkMpiAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixDarkMpiAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixDarkMpiAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixDarkMpiAsk.Make());
    CanvasNodeBuilder chixPrimaryPegBid(
      *GetPeggedBidOrderTaskNode(false)->Rename(
      "CHI-X Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    chixPrimaryPegBid.SetReadOnly("exec_inst", true);
    chixPrimaryPegBid.SetVisible("exec_inst", false);
    chixPrimaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixPrimaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixPrimaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixPrimaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixPrimaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixPrimaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixPrimaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixPrimaryPegBid.Make());
    CanvasNodeBuilder chixPrimaryPegAsk(
      *GetPeggedAskOrderTaskNode(false)->Rename(
      "CHI-X Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    chixPrimaryPegAsk.SetReadOnly("exec_inst", true);
    chixPrimaryPegAsk.SetVisible("exec_inst", false);
    chixPrimaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixPrimaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixPrimaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixPrimaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixPrimaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixPrimaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixPrimaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixPrimaryPegAsk.Make());
    CanvasNodeBuilder chixMidPegBid(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CHI-X Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    chixMidPegBid.SetReadOnly("exec_inst", true);
    chixMidPegBid.SetVisible("exec_inst", false);
    chixMidPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMidPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixMidPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixMidPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMidPegBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixMidPegBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMidPegBid.Make());
    CanvasNodeBuilder chixMidPegAsk(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CHI-X Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    chixMidPegAsk.SetReadOnly("exec_inst", true);
    chixMidPegAsk.SetVisible("exec_inst", false);
    chixMidPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMidPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixMidPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixMidPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMidPegAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixMidPegAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMidPegAsk.Make());
    CanvasNodeBuilder chixDarkMidPegBid(
      *GetPeggedBidOrderTaskNode(true)->Rename(
      "CHI-X Dark Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      std::make_unique<TextNode>("SMRTCXD")));
    chixDarkMidPegBid.SetReadOnly("exec_inst", true);
    chixDarkMidPegBid.SetVisible("exec_inst", false);
    chixDarkMidPegBid.SetVisible("ex_destination", false);
    chixDarkMidPegBid.SetReadOnly("ex_destination", true);
    chixDarkMidPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixDarkMidPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixDarkMidPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixDarkMidPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixDarkMidPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixDarkMidPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixDarkMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixDarkMidPegBid.Make());
    CanvasNodeBuilder chixDarkMidPegAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename(
      "CHI-X Dark Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      std::make_unique<TextNode>("SMRTCXD")));
    chixDarkMidPegAsk.SetReadOnly("exec_inst", true);
    chixDarkMidPegAsk.SetVisible("exec_inst", false);
    chixDarkMidPegAsk.SetVisible("ex_destination", false);
    chixDarkMidPegAsk.SetReadOnly("ex_destination", true);
    chixDarkMidPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixDarkMidPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixDarkMidPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixDarkMidPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixDarkMidPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixDarkMidPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixDarkMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixDarkMidPegAsk.Make());
    CanvasNodeBuilder chixMarketPegBid(
      *GetPeggedBidOrderTaskNode(true)->Rename(
      "CHI-X Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    chixMarketPegBid.SetReadOnly("exec_inst", true);
    chixMarketPegBid.SetVisible("exec_inst", false);
    chixMarketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMarketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixMarketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixMarketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMarketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixMarketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixMarketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMarketPegBid.Make());
    CanvasNodeBuilder chixMarketPegAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename(
      "CHI-X Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    chixMarketPegAsk.SetReadOnly("exec_inst", true);
    chixMarketPegAsk.SetVisible("exec_inst", false);
    chixMarketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMarketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    chixMarketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    chixMarketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMarketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    chixMarketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    chixMarketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMarketPegAsk.Make());
    CanvasNodeBuilder chixMultiMidPegBid(
      *GetPeggedBidOrderTaskNode(true)->Rename(
      "CHI-X Multi-Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      std::make_unique<TextNode>("MULTIDARK-YCM")));
    chixMultiMidPegBid.SetReadOnly("exec_inst", true);
    chixMultiMidPegBid.SetVisible("exec_inst", false);
    chixMultiMidPegBid.SetReadOnly("ex_destination", true);
    chixMultiMidPegBid.SetVisible("ex_destination", false);
    chixMultiMidPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMultiMidPegBid.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixMultiMidPegBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixMultiMidPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMultiMidPegBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixMultiMidPegBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixMultiMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMultiMidPegBid.Make());
    CanvasNodeBuilder chixMultiMidPegAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename(
      "CHI-X Multi-Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      std::make_unique<TextNode>("MULTIDARK-YCM")));
    chixMultiMidPegAsk.SetReadOnly("exec_inst", true);
    chixMultiMidPegAsk.SetVisible("exec_inst", false);
    chixMultiMidPegAsk.SetReadOnly("ex_destination", true);
    chixMultiMidPegAsk.SetVisible("ex_destination", false);
    chixMultiMidPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMultiMidPegAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    chixMultiMidPegAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    chixMultiMidPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMultiMidPegAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    chixMultiMidPegAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    chixMultiMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMultiMidPegAsk.Make());
  }

  void PopulateCseOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "CSE Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "CSE Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "CSE Market Bid"));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "CSE Market Ask"));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename(
      "CSE Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename(
      "CSE Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CSE Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CSE Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CSE Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CSE Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateCse2Orders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    auto limitBid = CanvasNodeBuilder(
      *GetLimitBidOrderTaskNode()->Rename("CSE2 Limit Bid")->AddField(
        "max_floor", 111, LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    auto limitAsk = CanvasNodeBuilder(*GetLimitAskOrderTaskNode()->Rename(
      "CSE2 Limit Ask")->AddField("max_floor", 111,
        LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    auto marketBid = CanvasNodeBuilder(
      *GetMarketBidOrderTaskNode()->Rename("CSE2 Market Bid"));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    auto marketAsk = CanvasNodeBuilder(
      *GetMarketAskOrderTaskNode()->Rename("CSE2 Market Ask"));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    auto buy =
      CanvasNodeBuilder(*GetMarketBidOrderTaskNode()->Rename("CSE2 Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    auto sell =
      CanvasNodeBuilder(*GetMarketAskOrderTaskNode()->Rename("CSE2 Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    auto darkLimitBid = CanvasNodeBuilder(
      *GetLimitBidOrderTaskNode()->Rename("CSE2 Dark Limit Bid")->AddField(
        "max_floor", 111, std::make_unique<IntegerNode>(0)));
    darkLimitBid.SetVisible("max_floor", false);
    darkLimitBid.SetReadOnly("max_floor", true);
    darkLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    darkLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    darkLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    darkLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    darkLimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    darkLimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    darkLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(darkLimitBid.Make());
    auto darkLimitAsk = CanvasNodeBuilder(
      *GetLimitAskOrderTaskNode()->Rename("CSE2 Dark Limit Ask")->AddField(
        "max_floor", 111, std::make_unique<IntegerNode>(0)));
    darkLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    darkLimitAsk.SetVisible("max_floor", false);
    darkLimitAsk.SetReadOnly("max_floor", true);
    darkLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    darkLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    darkLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    darkLimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    darkLimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    darkLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(darkLimitAsk.Make());
    auto midPegBid = CanvasNodeBuilder(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CSE2 Mid Peg Bid")->AddField(
        "exec_inst", 18, std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    auto midPegAsk = CanvasNodeBuilder(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CSE2 Mid Peg Ask")->AddField(
        "exec_inst", 18, std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    auto marketPegBid = CanvasNodeBuilder(
      *GetPeggedBidOrderTaskNode(true)->Rename("CSE2 Market Peg Bid")->AddField(
        "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
          "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    auto marketPegAsk = CanvasNodeBuilder(
      *GetPeggedAskOrderTaskNode(true)->Rename("CSE2 Market Peg Ask")->AddField(
        "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
          "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CSE2()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateCx2Orders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder cx2LimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "CX2 Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    cx2LimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2LimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2LimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2LimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2LimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    cx2LimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    cx2LimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2LimitBid.Make());
    CanvasNodeBuilder cx2LimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "CX2 Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    cx2LimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2LimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2LimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2LimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2LimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    cx2LimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    cx2LimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2LimitAsk.Make());
    CanvasNodeBuilder cx2MarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "CX2 Market Bid"));
    cx2MarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2MarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2MarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MarketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    cx2MarketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    cx2MarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MarketBid.Make());
    CanvasNodeBuilder cx2MarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "CX2 Market Ask"));
    cx2MarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2MarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2MarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MarketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    cx2MarketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    cx2MarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MarketAsk.Make());
    CanvasNodeBuilder cx2Buy(*GetMarketBidOrderTaskNode()->Rename("CX2 Buy"));
    cx2Buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    cx2Buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2Buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2Buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2Buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2Buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    cx2Buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    cx2Buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2Buy.Make());
    CanvasNodeBuilder cx2Sell(*GetMarketAskOrderTaskNode()->Rename(
      "CX2 Sell"));
    cx2Sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    cx2Sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2Sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2Sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2Sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2Sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    cx2Sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    cx2Sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2Sell.Make());
    CanvasNodeBuilder cx2PrimaryPegBid(
      *GetPeggedBidOrderTaskNode(true)->Rename(
      "CX2 Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    cx2PrimaryPegBid.SetReadOnly("exec_inst", true);
    cx2PrimaryPegBid.SetVisible("exec_inst", false);
    cx2PrimaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2PrimaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    cx2PrimaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    cx2PrimaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2PrimaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    cx2PrimaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    cx2PrimaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2PrimaryPegBid.Make());
    CanvasNodeBuilder cx2PrimaryPegAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename(
      "CX2 Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    cx2PrimaryPegAsk.SetReadOnly("exec_inst", true);
    cx2PrimaryPegAsk.SetVisible("exec_inst", false);
    cx2PrimaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2PrimaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    cx2PrimaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    cx2PrimaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2PrimaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    cx2PrimaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    cx2PrimaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2PrimaryPegAsk.Make());
    CanvasNodeBuilder cx2MidPegBid(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CX2 Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    cx2MidPegBid.SetReadOnly("exec_inst", true);
    cx2MidPegBid.SetVisible("exec_inst", false);
    cx2MidPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MidPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2MidPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2MidPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MidPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    cx2MidPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    cx2MidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MidPegBid.Make());
    CanvasNodeBuilder cx2MidPegAsk(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CX2 Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    cx2MidPegAsk.SetReadOnly("exec_inst", true);
    cx2MidPegAsk.SetVisible("exec_inst", false);
    cx2MidPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MidPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2MidPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2MidPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MidPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    cx2MidPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    cx2MidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MidPegAsk.Make());
    CanvasNodeBuilder cx2MarketPegBid(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CX2 Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    cx2MarketPegBid.SetReadOnly("exec_inst", true);
    cx2MarketPegBid.SetVisible("exec_inst", false);
    cx2MarketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MarketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2MarketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2MarketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MarketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    cx2MarketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    cx2MarketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MarketPegBid.Make());
    CanvasNodeBuilder cx2MarketPegAsk(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CX2 Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    cx2MarketPegAsk.SetReadOnly("exec_inst", true);
    cx2MarketPegAsk.SetVisible("exec_inst", false);
    cx2MarketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MarketPegAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    cx2MarketPegAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    cx2MarketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MarketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    cx2MarketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    cx2MarketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MarketPegAsk.Make());
  }

  void PopulateHkexOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "HKEX Limit Bid"));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::HKEX()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "HKEX Limit Ask"));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::HKEX()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "HKEX Market Bid"));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::HKEX()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "HKEX Market Ask"));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::HKEX()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
  }

  void PopulateLynxOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder lynxLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "Lynx Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    lynxLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    lynxLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    lynxLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxLimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    lynxLimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    lynxLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxLimitBid.Make());
    CanvasNodeBuilder lynxLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "Lynx Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    lynxLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    lynxLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    lynxLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxLimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    lynxLimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    lynxLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxLimitAsk.Make());
    CanvasNodeBuilder lynxMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "Lynx Market Bid"));
    lynxMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    lynxMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    lynxMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxMarketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    lynxMarketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    lynxMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxMarketBid.Make());
    CanvasNodeBuilder lynxMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "Lynx Market Ask"));
    lynxMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    lynxMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    lynxMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxMarketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    lynxMarketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    lynxMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxMarketAsk.Make());
    CanvasNodeBuilder lynxBuy(*GetMarketBidOrderTaskNode()->Rename(
      "Lynx Buy"));
    lynxBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    lynxBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    lynxBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    lynxBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    lynxBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    lynxBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxBuy.Make());
    CanvasNodeBuilder lynxSell(*GetMarketAskOrderTaskNode()->Rename(
      "Lynx Sell"));
    lynxSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    lynxSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    lynxSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    lynxSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    lynxSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    lynxSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxSell.Make());
  }

  void PopulateMatchNowLpOrders(
      vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder matnlpLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "MATCH Now LP Limit Bid"));
    matnlpLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpLimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    matnlpLimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    matnlpLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpLimitBid.Make());
    CanvasNodeBuilder matnlpLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "MATCH Now LP Limit Ask"));
    matnlpLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpLimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    matnlpLimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    matnlpLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpLimitAsk.Make());
    CanvasNodeBuilder matnlpAttBid(*GetLimitBidOrderTaskNode()->Rename(
      "MATCH Now LP At-The-Touch Bid")->AddField(
      "constraints", 6005, std::make_unique<TextNode>("PAG")));
    matnlpAttBid.SetVisible("constraints", false);
    matnlpAttBid.SetReadOnly("constraints", true);
    matnlpAttBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpAttBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpAttBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpAttBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpAttBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnlpAttBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnlpAttBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpAttBid.Make());
    CanvasNodeBuilder matnlpAttAsk(*GetLimitAskOrderTaskNode()->Rename(
      "MATCH Now LP At-The-Touch Ask")->AddField(
      "constraints", 6005, std::make_unique<TextNode>("PAG")));
    matnlpAttAsk.SetVisible("constraints", false);
    matnlpAttAsk.SetReadOnly("constraints", true);
    matnlpAttAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpAttAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpAttAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpAttAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpAttAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnlpAttAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnlpAttAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpAttAsk.Make());
    CanvasNodeBuilder matnlpMpiBid(*GetLimitBidOrderTaskNode()->Rename(
      "MATCH Now LP MPI Bid")->AddField(
      "constraints", 6005, std::make_unique<TextNode>("PMI")));
    matnlpMpiBid.SetVisible("constraints", false);
    matnlpMpiBid.SetReadOnly("constraints", true);
    matnlpMpiBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpMpiBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpMpiBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpMpiBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpMpiBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnlpMpiBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnlpMpiBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpMpiBid.Make());
    CanvasNodeBuilder matnlpMpiAsk(*GetLimitAskOrderTaskNode()->Rename(
      "MATCH Now LP MPI Ask")->AddField(
      "constraints", 6005, std::make_unique<TextNode>("PMI")));
    matnlpMpiAsk.SetVisible("constraints", false);
    matnlpMpiAsk.SetReadOnly("constraints", true);
    matnlpMpiAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpMpiAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpMpiAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpMpiAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpMpiAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnlpMpiAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnlpMpiAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpMpiAsk.Make());
    CanvasNodeBuilder matnlpMarketBid(
      *GetMarketBidOrderTaskNode()->Rename("MATCH Now LP Market Bid"));
    matnlpMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpMarketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    matnlpMarketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    matnlpMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpMarketBid.Make());
    CanvasNodeBuilder matnlpMarketAsk(
      *GetMarketAskOrderTaskNode()->Rename("MATCH Now LP Market Ask"));
    matnlpMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpMarketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    matnlpMarketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    matnlpMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpMarketAsk.Make());
    CanvasNodeBuilder matnlpBuy(*GetMarketBidOrderTaskNode()->Rename(
      "MATCH Now LP Buy"));
    matnlpBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    matnlpBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnlpBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnlpBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpBuy.Make());
    CanvasNodeBuilder matnlpSell(
      *GetMarketAskOrderTaskNode()->Rename("MATCH Now LP Sell"));
    matnlpSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    matnlpSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnlpSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnlpSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnlpSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnlpSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpSell.Make());
  }

  void PopulateMatchNowMfOrders(
      vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder matnMarketFlowLimitBid(
      *GetLimitBidOrderTaskNode()->Rename("MATCH Now MF Limit Bid"));
    matnMarketFlowLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnMarketFlowLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    matnMarketFlowLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    matnMarketFlowLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnMarketFlowLimitBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnMarketFlowLimitBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnMarketFlowLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnMarketFlowLimitBid.Make());
    CanvasNodeBuilder matnMarketFlowLimitAsk(
      *GetLimitAskOrderTaskNode()->Rename("MATCH Now MF Limit Ask"));
    matnMarketFlowLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnMarketFlowLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    matnMarketFlowLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    matnMarketFlowLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnMarketFlowLimitAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnMarketFlowLimitAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnMarketFlowLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnMarketFlowLimitAsk.Make());
    CanvasNodeBuilder matnMarketFlowMarketBid(
      *GetMarketBidOrderTaskNode()->Rename("MATCH Now MF Market Bid"));
    matnMarketFlowMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnMarketFlowMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    matnMarketFlowMarketBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnMarketFlowMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnMarketFlowMarketBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnMarketFlowMarketBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnMarketFlowMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnMarketFlowMarketBid.Make());
    CanvasNodeBuilder matnMarketFlowMarketAsk(
      *GetMarketAskOrderTaskNode()->Rename("MATCH Now MF Market Ask"));
    matnMarketFlowMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnMarketFlowMarketAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnMarketFlowMarketAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnMarketFlowMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnMarketFlowMarketAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnMarketFlowMarketAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnMarketFlowMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnMarketFlowMarketAsk.Make());
    CanvasNodeBuilder matnmfBuy(*GetMarketBidOrderTaskNode()->Rename(
      "MATCH Now MF Buy"));
    matnmfBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    matnmfBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnmfBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnmfBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnmfBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnmfBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnmfBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnmfBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnmfBuy.Make());
    CanvasNodeBuilder matnmfSell(
      *GetMarketAskOrderTaskNode()->Rename("MATCH Now MF Sell"));
    matnmfSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    matnmfSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnmfSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    matnmfSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    matnmfSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnmfSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    matnmfSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    matnmfSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnmfSell.Make());
  }

  void PopulateNeoeOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder litLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "NEO Lit Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    litLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    litLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    litLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litLimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    litLimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    litLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litLimitBid.Make());
    CanvasNodeBuilder litLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NEO Lit Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    litLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    litLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    litLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litLimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    litLimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    litLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litLimitAsk.Make());
    CanvasNodeBuilder litMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "NEO Lit Market Bid"));
    litMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    litMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    litMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litMarketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    litMarketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    litMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litMarketBid.Make());
    CanvasNodeBuilder litMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NEO Lit Market Ask"));
    litMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    litMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    litMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litMarketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    litMarketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    litMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litMarketAsk.Make());
    CanvasNodeBuilder litBuy(*GetMarketBidOrderTaskNode()->Rename(
      "NEO Lit Buy"));
    litBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    litBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    litBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    litBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    litBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    litBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litBuy.Make());
    CanvasNodeBuilder litSell(*GetMarketAskOrderTaskNode()->Rename(
      "NEO Lit Sell"));
    litSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    litSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    litSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    litSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    litSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    litSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litSell.Make());
    CanvasNodeBuilder litMidPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NEOE Lit Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    litMidPegBid.SetReadOnly("exec_inst", true);
    litMidPegBid.SetVisible("exec_inst", false);
    litMidPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litMidPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    litMidPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    litMidPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litMidPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    litMidPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    litMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litMidPegBid.Make());
    CanvasNodeBuilder litMidPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NEOE Lit Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    litMidPegAsk.SetReadOnly("exec_inst", true);
    litMidPegAsk.SetVisible("exec_inst", false);
    litMidPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litMidPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    litMidPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    litMidPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litMidPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    litMidPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    litMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litMidPegAsk.Make());
    CanvasNodeBuilder neoLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "NEO Book Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("N")));
    neoLimitBid.SetReadOnly("ex_destination", true);
    neoLimitBid.SetVisible("ex_destination", false);
    neoLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoLimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoLimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoLimitBid.Make());
    CanvasNodeBuilder neoLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NEO Book Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("N")));
    neoLimitAsk.SetReadOnly("ex_destination", true);
    neoLimitAsk.SetVisible("ex_destination", false);
    neoLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoLimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoLimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoLimitAsk.Make());
    CanvasNodeBuilder neoMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "NEO Book Market Bid")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoMarketBid.SetReadOnly("ex_destination", true);
    neoMarketBid.SetVisible("ex_destination", false);
    neoMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoMarketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoMarketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoMarketBid.Make());
    CanvasNodeBuilder neoMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NEO Book Market Ask")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoMarketAsk.SetReadOnly("ex_destination", true);
    neoMarketAsk.SetVisible("ex_destination", false);
    neoMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoMarketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoMarketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoMarketAsk.Make());
    CanvasNodeBuilder neoBuy(*GetMarketBidOrderTaskNode()->Rename(
      "NEO Book Buy")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoBuy.SetReadOnly("ex_destination", true);
    neoBuy.SetVisible("ex_destination", false);
    neoBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    neoBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoBuy.Make());
    CanvasNodeBuilder neoSell(*GetMarketAskOrderTaskNode()->Rename(
      "NEO Book Sell")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoSell.SetReadOnly("ex_destination", true);
    neoSell.SetVisible("ex_destination", false);
    neoSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    neoSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoSell.Make());
    CanvasNodeBuilder neoIocBid(*GetLimitBidOrderTaskNode()->Rename(
      "NEO Book IOC Bid")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoIocBid.SetReadOnly("ex_destination", true);
    neoIocBid.SetVisible("ex_destination", false);
    neoIocBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoIocBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoIocBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoIocBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::IOC)));
    neoIocBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoIocBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoIocBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoIocBid.Make());
    CanvasNodeBuilder neoIocAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NEO Book IOC Ask")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoIocAsk.SetReadOnly("ex_destination", true);
    neoIocAsk.SetVisible("ex_destination", false);
    neoIocAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoIocAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoIocAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoIocAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::IOC)));
    neoIocAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoIocAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoIocAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoIocAsk.Make());
    CanvasNodeBuilder neoFokBid(*GetLimitBidOrderTaskNode()->Rename(
      "NEO Book FOK Bid")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoFokBid.SetReadOnly("ex_destination", true);
    neoFokBid.SetVisible("ex_destination", false);
    neoFokBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoFokBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoFokBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoFokBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::FOK)));
    neoFokBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoFokBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoFokBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoFokBid.Make());
    CanvasNodeBuilder neoFokAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NEO Book FOK Ask")->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoFokAsk.SetReadOnly("ex_destination", true);
    neoFokAsk.SetVisible("ex_destination", false);
    neoFokAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoFokAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoFokAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoFokAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::FOK)));
    neoFokAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoFokAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoFokAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoFokAsk.Make());
    CanvasNodeBuilder neoMidPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NEOE Book Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoMidPegBid.SetReadOnly("ex_destination", true);
    neoMidPegBid.SetVisible("ex_destination", false);
    neoMidPegBid.SetReadOnly("exec_inst", true);
    neoMidPegBid.SetVisible("exec_inst", false);
    neoMidPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoMidPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoMidPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoMidPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoMidPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoMidPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoMidPegBid.Make());
    CanvasNodeBuilder neoMidPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NEOE Book Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      std::make_unique<TextNode>("N")));
    neoMidPegAsk.SetReadOnly("ex_destination", true);
    neoMidPegAsk.SetVisible("ex_destination", false);
    neoMidPegAsk.SetReadOnly("exec_inst", true);
    neoMidPegAsk.SetVisible("exec_inst", false);
    neoMidPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoMidPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    neoMidPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    neoMidPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoMidPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    neoMidPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    neoMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoMidPegAsk.Make());
  }

  void PopulateOmegaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder omegaLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "Omega Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    omegaLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    omegaLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    omegaLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaLimitBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    omegaLimitBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    omegaLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaLimitBid.Make());
    CanvasNodeBuilder omegaLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "Omega Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    omegaLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    omegaLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    omegaLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaLimitAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    omegaLimitAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    omegaLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaLimitAsk.Make());
    CanvasNodeBuilder omegaMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "Omega Market Bid"));
    omegaMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    omegaMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    omegaMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaMarketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    omegaMarketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    omegaMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaMarketBid.Make());
    CanvasNodeBuilder omegaMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "Omega Market Ask"));
    omegaMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    omegaMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    omegaMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaMarketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    omegaMarketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    omegaMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaMarketAsk.Make());
    CanvasNodeBuilder omegaBuy(*GetMarketBidOrderTaskNode()->Rename(
      "Omega Buy"));
    omegaBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    omegaBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    omegaBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    omegaBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    omegaBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    omegaBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaBuy.Make());
    CanvasNodeBuilder omegaSell(*GetMarketAskOrderTaskNode()->Rename(
      "Omega Sell"));
    omegaSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    omegaSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    omegaSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    omegaSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    omegaSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    omegaSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaSell.Make());
  }

  void PopulatePureOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder pureLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "Pure Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    pureLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    pureLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    pureLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureLimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    pureLimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    pureLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureLimitBid.Make());
    CanvasNodeBuilder pureLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "Pure Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    pureLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    pureLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    pureLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureLimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    pureLimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    pureLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureLimitAsk.Make());
    CanvasNodeBuilder pureMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "Pure Market Bid"));
    pureMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    pureMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    pureMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureMarketBid.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    pureMarketBid.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    pureMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureMarketBid.Make());
    CanvasNodeBuilder pureMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "Pure Market Ask"));
    pureMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    pureMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    pureMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureMarketAsk.SetVisible(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    pureMarketAsk.SetReadOnly(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    pureMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureMarketAsk.Make());
    CanvasNodeBuilder pureBuy(*GetMarketBidOrderTaskNode()->Rename(
      "Pure Buy"));
    pureBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    pureBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    pureBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    pureBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    pureBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    pureBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureBuy.Make());
    CanvasNodeBuilder pureSell(*GetMarketAskOrderTaskNode()->Rename(
      "Pure Sell"));
    pureSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    pureSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    pureSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    pureSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    pureSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    pureSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureSell.Make());
  }

  void PopulateTsxOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder tsxLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "TSX Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "long_life", 7735, std::make_unique<TextNode>("Y")));
    tsxLimitBid.SetReadOnly("long_life", true);
    tsxLimitBid.SetVisible("long_life", false);
    tsxLimitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxLimitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    tsxLimitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    tsxLimitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxLimitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    tsxLimitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    tsxLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxLimitBid.Make());
    CanvasNodeBuilder tsxLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "TSX Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "long_life", 7735, std::make_unique<TextNode>("Y")));
    tsxLimitAsk.SetReadOnly("long_life", true);
    tsxLimitAsk.SetVisible("long_life", false);
    tsxLimitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxLimitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    tsxLimitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    tsxLimitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxLimitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    tsxLimitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    tsxLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxLimitAsk.Make());
    CanvasNodeBuilder tsxMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "TSX Market Bid"));
    tsxMarketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxMarketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    tsxMarketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    tsxMarketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxMarketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    tsxMarketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    tsxMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxMarketBid.Make());
    CanvasNodeBuilder tsxMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "TSX Market Ask"));
    tsxMarketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxMarketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    tsxMarketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    tsxMarketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxMarketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    tsxMarketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    tsxMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxMarketAsk.Make());
    CanvasNodeBuilder tsxBuy(*GetMarketBidOrderTaskNode()->Rename("TSX Buy"));
    tsxBuy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    tsxBuy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxBuy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    tsxBuy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    tsxBuy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxBuy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    tsxBuy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    tsxBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxBuy.Make());
    CanvasNodeBuilder tsxSell(*GetMarketAskOrderTaskNode()->Rename(
      "TSX Sell"));
    tsxSell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    tsxSell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxSell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    tsxSell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    tsxSell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxSell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    tsxSell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    tsxSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxSell.Make());
    CanvasNodeBuilder tsxLimitOnCloseBid(*GetLimitBidOrderTaskNode()->Rename(
      "TSX Limit On Close Bid"));
    tsxLimitOnCloseBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxLimitOnCloseBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    tsxLimitOnCloseBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    tsxLimitOnCloseBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    tsxLimitOnCloseBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    tsxLimitOnCloseBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    tsxLimitOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxLimitOnCloseBid.Make());
    CanvasNodeBuilder tsxLimitOnCloseAsk(*GetLimitAskOrderTaskNode()->Rename(
      "TSX Limit On Close Ask"));
    tsxLimitOnCloseAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxLimitOnCloseAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    tsxLimitOnCloseAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    tsxLimitOnCloseAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    tsxLimitOnCloseAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    tsxLimitOnCloseAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    tsxLimitOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxLimitOnCloseAsk.Make());
    CanvasNodeBuilder tsxMarketOnCloseBid(*GetMarketBidOrderTaskNode()->Rename(
      "TSX Market On Close Bid"));
    tsxMarketOnCloseBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxMarketOnCloseBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    tsxMarketOnCloseBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    tsxMarketOnCloseBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    tsxMarketOnCloseBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    tsxMarketOnCloseBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    tsxMarketOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxMarketOnCloseBid.Make());
    CanvasNodeBuilder tsxMarketOnCloseAsk(*GetMarketAskOrderTaskNode()->Rename(
      "TSX Market On Close Ask"));
    tsxMarketOnCloseAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxMarketOnCloseAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    tsxMarketOnCloseAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    tsxMarketOnCloseAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    tsxMarketOnCloseAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    tsxMarketOnCloseAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    tsxMarketOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxMarketOnCloseAsk.Make());
    CanvasNodeBuilder tsxDarkMidPointBid(
      *GetPeggedBidOrderTaskNode(true)->Rename("TSX Dark Mid-Point Bid"));
    tsxDarkMidPointBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxDarkMidPointBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    tsxDarkMidPointBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    tsxDarkMidPointBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxDarkMidPointBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    tsxDarkMidPointBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    tsxDarkMidPointBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxDarkMidPointBid.Make());
    CanvasNodeBuilder tsxDarkMidPointAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename("TSX Dark Mid-Point Ask"));
    tsxDarkMidPointAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxDarkMidPointAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    tsxDarkMidPointAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    tsxDarkMidPointAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxDarkMidPointAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    tsxDarkMidPointAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    tsxDarkMidPointAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxDarkMidPointAsk.Make());
  }

  void PopulateAmexOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "AMEX Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "AMEX Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "AMEX Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "AMEX Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("AMEX Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("AMEX Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "AMEX Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "AMEX Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "AMEX Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "AMEX Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "AMEX Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "AMEX Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateArcaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "ARCA Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "ARCA Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "ARCA Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "ARCA Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("ARCA Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("ARCA Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ARCA Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ARCA Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ARCA Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ARCA Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ARCA Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ARCA Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateBatsOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "BATS Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "BATS Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "BATS Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "BATS Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("BATS Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("BATS Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATS Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATS Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATS Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATS Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATS Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATS Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATS()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateBatyOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "BATY Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "BATY Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "BATY Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "BATY Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("BATY Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("BATY Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATY Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATY Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATY Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATY Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATY Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATY Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::BATY()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateEdgaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "EDGA Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "EDGA Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "EDGA Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "EDGA Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("EDGA Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("EDGA Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGA Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGA Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGA Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGA Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGA Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGA Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateEdgxOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "EDGX Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "EDGX Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "EDGX Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "EDGX Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("EDGX Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("EDGX Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGX Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGX Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGX Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGX Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGX Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGX Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateNyseOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "NYSE Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NYSE Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "NYSE Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NYSE Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("NYSE Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("NYSE Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NYSE Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NYSE Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NYSE Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NYSE Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NYSE Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NYSE Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
  }

  void PopulateNyseOpenCloseOrders(
      vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitOnOpenBid(*GetLimitBidOrderTaskNode()->Rename(
      "NYSE Limit On Open Bid"));
    limitOnOpenBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitOnOpenBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitOnOpenBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitOnOpenBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    limitOnOpenBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    limitOnOpenBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    limitOnOpenBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnOpenBid.Make());
    CanvasNodeBuilder limitOnOpenAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NYSE Limit On Open Ask"));
    limitOnOpenAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitOnOpenAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitOnOpenAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitOnOpenAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    limitOnOpenAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    limitOnOpenAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    limitOnOpenAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnOpenAsk.Make());
    CanvasNodeBuilder marketOnOpenBid(*GetMarketBidOrderTaskNode()->Rename(
      "NYSE Market On Open Bid"));
    marketOnOpenBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketOnOpenBid.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketOnOpenBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketOnOpenBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    marketOnOpenBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketOnOpenBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketOnOpenBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnOpenBid.Make());
    CanvasNodeBuilder marketOnOpenAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NYSE Market On Open Ask"));
    marketOnOpenAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketOnOpenAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketOnOpenAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketOnOpenAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    marketOnOpenAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketOnOpenAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketOnOpenAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnOpenAsk.Make());
    CanvasNodeBuilder limitOnCloseBid(*GetLimitBidOrderTaskNode()->Rename(
      "NYSE Limit On Close Bid"));
    limitOnCloseBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitOnCloseBid.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitOnCloseBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitOnCloseBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    limitOnCloseBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    limitOnCloseBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    limitOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnCloseBid.Make());
    CanvasNodeBuilder limitOnCloseAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NYSE Limit On Close Ask"));
    limitOnCloseAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitOnCloseAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitOnCloseAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitOnCloseAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    limitOnCloseAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    limitOnCloseAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    limitOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnCloseAsk.Make());
    CanvasNodeBuilder marketOnCloseBid(*GetMarketBidOrderTaskNode()->Rename(
      "NYSE Market On Close Bid"));
    marketOnCloseBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketOnCloseBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    marketOnCloseBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    marketOnCloseBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    marketOnCloseBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketOnCloseBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnCloseBid.Make());
    CanvasNodeBuilder marketOnCloseAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NYSE Market On Close Ask"));
    marketOnCloseAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketOnCloseAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    marketOnCloseAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    marketOnCloseAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    marketOnCloseAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketOnCloseAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnCloseAsk.Make());
  }

  void PopulateNasdaqOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "NASDAQ Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NASDAQ Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "NASDAQ Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NASDAQ Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("NASDAQ Buy"));
    buy.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    buy.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    buy.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    buy.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    buy.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Make());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename(
      "NASDAQ Sell"));
    sell.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    sell.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    sell.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    sell.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    sell.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Make());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NASDAQ Primary Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    primaryPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Make());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NASDAQ Primary Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    primaryPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    primaryPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Make());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NASDAQ Mid Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    midPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Make());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NASDAQ Mid Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    midPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    midPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    midPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Make());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NASDAQ Market Peg Bid")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketPegBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Make());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NASDAQ Market Peg Ask")->AddField("exec_inst", 18,
      std::make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      std::make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketPegAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketPegAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketPegAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Make());
    CanvasNodeBuilder limitOnOpenBid(*GetLimitBidOrderTaskNode()->Rename(
      "NASDAQ Limit On Open Bid"));
    limitOnOpenBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitOnOpenBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitOnOpenBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitOnOpenBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    limitOnOpenBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    limitOnOpenBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    limitOnOpenBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnOpenBid.Make());
    CanvasNodeBuilder limitOnOpenAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NASDAQ Limit On Open Ask"));
    limitOnOpenAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitOnOpenAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitOnOpenAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitOnOpenAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    limitOnOpenAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    limitOnOpenAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    limitOnOpenAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnOpenAsk.Make());
    CanvasNodeBuilder marketOnOpenBid(*GetMarketBidOrderTaskNode()->Rename(
      "NASDAQ Market On Open Bid"));
    marketOnOpenBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketOnOpenBid.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketOnOpenBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketOnOpenBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    marketOnOpenBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketOnOpenBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketOnOpenBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnOpenBid.Make());
    CanvasNodeBuilder marketOnOpenAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NASDAQ Market On Open Ask"));
    marketOnOpenAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketOnOpenAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketOnOpenAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketOnOpenAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    marketOnOpenAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketOnOpenAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketOnOpenAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnOpenAsk.Make());
    CanvasNodeBuilder limitOnCloseBid(*GetLimitBidOrderTaskNode()->Rename(
      "NASDAQ Limit On Close Bid"));
    limitOnCloseBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitOnCloseBid.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitOnCloseBid.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitOnCloseBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    limitOnCloseBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    limitOnCloseBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    limitOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnCloseBid.Make());
    CanvasNodeBuilder limitOnCloseAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NASDAQ Limit On Close Ask"));
    limitOnCloseAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitOnCloseAsk.SetVisible(
      SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitOnCloseAsk.SetReadOnly(
      SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitOnCloseAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    limitOnCloseAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    limitOnCloseAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    limitOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnCloseAsk.Make());
    CanvasNodeBuilder marketOnCloseBid(*GetMarketBidOrderTaskNode()->Rename(
      "NASDAQ Market On Close Bid"));
    marketOnCloseBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketOnCloseBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    marketOnCloseBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    marketOnCloseBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    marketOnCloseBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketOnCloseBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnCloseBid.Make());
    CanvasNodeBuilder marketOnCloseAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NASDAQ Market On Close Ask"));
    marketOnCloseAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketOnCloseAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY,
      false);
    marketOnCloseAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY,
      true);
    marketOnCloseAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    marketOnCloseAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      false);
    marketOnCloseAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      true);
    marketOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnCloseAsk.Make());
  }

  void PopulateOseOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "OSE Limit Bid"));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OSE()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "OSE Limit Ask"));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OSE()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "OSE Market Bid"));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OSE()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "OSE Market Ask"));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::OSE()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
  }

  void PopulateTseOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "TSE Limit Bid"));
    limitBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSE()));
    limitBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Make());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "TSE Limit Ask"));
    limitAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSE()));
    limitAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    limitAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    limitAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Make());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "TSE Market Bid"));
    marketBid.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSE()));
    marketBid.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketBid.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketBid.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Make());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "TSE Market Ask"));
    marketAsk.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(DefaultDestinations::TSE()));
    marketAsk.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
    marketAsk.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    marketAsk.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Make());
  }

  std::unordered_map<MarketCode, vector<unique_ptr<const CanvasNode>>>
      SetupOrderTypes() {
    std::unordered_map<MarketCode, vector<unique_ptr<const CanvasNode>>>
      orderTypes;
    auto& asxOrderTypes = orderTypes[DefaultMarkets::ASX()];
    PopulateAsxOrders(asxOrderTypes);
    PopulateCxaOrders(asxOrderTypes);
    auto& arcxOrderTypes = orderTypes[DefaultMarkets::ARCX()];
    PopulateArcaOrders(arcxOrderTypes);
    PopulateBatsOrders(arcxOrderTypes);
    PopulateBatyOrders(arcxOrderTypes);
    PopulateEdgaOrders(arcxOrderTypes);
    PopulateEdgxOrders(arcxOrderTypes);
    PopulateNasdaqOrders(arcxOrderTypes);
    auto& asexOrderTypes = orderTypes[DefaultMarkets::ASEX()];
    PopulateAmexOrders(asexOrderTypes);
    PopulateArcaOrders(asexOrderTypes);
    PopulateBatsOrders(asexOrderTypes);
    PopulateBatyOrders(asexOrderTypes);
    PopulateEdgaOrders(asexOrderTypes);
    PopulateEdgxOrders(asexOrderTypes);
    PopulateNasdaqOrders(asexOrderTypes);
    auto& cseOrderTypes = orderTypes[DefaultMarkets::CSE()];
    PopulateChixOrders(cseOrderTypes);
    PopulateCseOrders(cseOrderTypes);
    PopulateCse2Orders(cseOrderTypes);
    PopulateCx2Orders(cseOrderTypes);
    PopulateLynxOrders(cseOrderTypes);
    PopulateMatchNowLpOrders(cseOrderTypes);
    PopulateMatchNowMfOrders(cseOrderTypes);
    PopulateNeoeOrders(cseOrderTypes);
    PopulateOmegaOrders(cseOrderTypes);
    PopulatePureOrders(cseOrderTypes);
    auto& hkexOrderTypes = orderTypes[DefaultMarkets::HKEX()];
    PopulateHkexOrders(hkexOrderTypes);
    auto& nasdaqOrderTypes = orderTypes[DefaultMarkets::NASDAQ()];
    PopulateArcaOrders(nasdaqOrderTypes);
    PopulateBatsOrders(nasdaqOrderTypes);
    PopulateBatyOrders(nasdaqOrderTypes);
    PopulateEdgaOrders(nasdaqOrderTypes);
    PopulateEdgxOrders(nasdaqOrderTypes);
    PopulateNasdaqOrders(nasdaqOrderTypes);
    auto& neoeOrderTypes = orderTypes[DefaultMarkets::NEOE()];
    PopulateMatchNowLpOrders(neoeOrderTypes);
    PopulateMatchNowMfOrders(neoeOrderTypes);
    PopulateNeoeOrders(neoeOrderTypes);
    auto& nyseOrderTypes = orderTypes[DefaultMarkets::NYSE()];
    PopulateArcaOrders(nyseOrderTypes);
    PopulateBatsOrders(nyseOrderTypes);
    PopulateBatyOrders(nyseOrderTypes);
    PopulateEdgaOrders(nyseOrderTypes);
    PopulateEdgxOrders(nyseOrderTypes);
    PopulateNyseOrders(nyseOrderTypes);
    PopulateNyseOpenCloseOrders(nyseOrderTypes);
    PopulateNasdaqOrders(nyseOrderTypes);
    auto& oseOrderTypes = orderTypes[DefaultMarkets::OSE()];
    PopulateOseOrders(oseOrderTypes);
    auto& tseOrderTypes = orderTypes[DefaultMarkets::TSE()];
    PopulateTseOrders(tseOrderTypes);
    auto& tsxOrderTypes = orderTypes[DefaultMarkets::TSX()];
    PopulateAlphaOrders(tsxOrderTypes);
    PopulateChixOrders(tsxOrderTypes);
    PopulateCse2Orders(tsxOrderTypes);
    PopulateCx2Orders(tsxOrderTypes);
    PopulateLynxOrders(tsxOrderTypes);
    PopulateMatchNowLpOrders(tsxOrderTypes);
    PopulateMatchNowMfOrders(tsxOrderTypes);
    PopulateNeoeOrders(tsxOrderTypes);
    PopulateOmegaOrders(tsxOrderTypes);
    PopulatePureOrders(tsxOrderTypes);
    PopulateTsxOrders(tsxOrderTypes);
    auto& tsxvOrderTypes = orderTypes[DefaultMarkets::TSXV()];
    PopulateAlphaOrders(tsxvOrderTypes);
    PopulateChixOrders(tsxvOrderTypes);
    PopulateCse2Orders(tsxvOrderTypes);
    PopulateCx2Orders(tsxvOrderTypes);
    PopulateLynxOrders(tsxvOrderTypes);
    PopulateMatchNowLpOrders(tsxvOrderTypes);
    PopulateMatchNowMfOrders(tsxvOrderTypes);
    PopulateNeoeOrders(tsxvOrderTypes);
    PopulateOmegaOrders(tsxvOrderTypes);
    PopulatePureOrders(tsxvOrderTypes);
    PopulateTsxOrders(tsxvOrderTypes);
    return orderTypes;
  }
}

SimplifiedKeyBindingsDialog::SimplifiedKeyBindingsDialog(
    Ref<UserProfile> userProfile, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_SimplifiedKeyBindingsDialog>()),
      m_userProfile(userProfile.Get()),
      m_keyBindings(m_userProfile->GetKeyBindings()) {
  m_ui->setupUi(this);
  m_ui->m_interactionsWidget->Initialize(Ref(*m_userProfile));
  m_orderTypes = SetupOrderTypes();
  auto entries = m_userProfile->GetMarketDatabase().GetEntries();
  std::sort(entries.begin(), entries.end(),
    [&] (const auto& left, const auto& right) {
      return left.m_displayName < right.m_displayName;
    });
  for(const auto& market : entries) {
    if(m_orderTypes.find(market.m_code) != m_orderTypes.end()) {
      m_ui->m_taskMarketComboBox->addItem(
        QString::fromStdString(market.m_displayName));
    }
  }
  m_ui->m_taskBindingsTable->setHorizontalHeaderLabels(
    QStringList() << tr("Order Type") << tr("Key"));
  m_ui->m_taskBindingsTable->setColumnWidth(0, 220);
  QFontMetrics taskMetrics(m_ui->m_taskBindingsTable->font());
  m_ui->m_taskBindingsTable->verticalHeader()->setDefaultSectionSize(
    taskMetrics.height() + 4);
  m_ui->m_taskBindingsTable->installEventFilter(this);
  m_ui->m_cancelsBindingsTable->setHorizontalHeaderLabels(
    QStringList() << tr("Cancel Option") << tr("Key"));
  m_ui->m_cancelsBindingsTable->setColumnWidth(0, 220);
  QFontMetrics cancelMetrics(m_ui->m_cancelsBindingsTable->font());
  m_ui->m_cancelsBindingsTable->verticalHeader()->setDefaultSectionSize(
    cancelMetrics.height() + 4);
  m_ui->m_cancelsBindingsTable->installEventFilter(this);
  connect(m_ui->m_taskMarketComboBox, static_cast<void (QComboBox::*)(int)>(
    &QComboBox::currentIndexChanged), this,
    &SimplifiedKeyBindingsDialog::OnMarketChanged);
  connect(m_ui->m_taskClearButton, &QPushButton::clicked, this,
    &SimplifiedKeyBindingsDialog::OnClearTasksButton);
  connect(m_ui->m_clearCancelsButton, &QPushButton::clicked, this,
    &SimplifiedKeyBindingsDialog::OnClearCancelsButton);
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &SimplifiedKeyBindingsDialog::OnOkButton);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &SimplifiedKeyBindingsDialog::reject);
  connect(m_ui->m_applyButton, &QPushButton::clicked, this,
    &SimplifiedKeyBindingsDialog::OnApplyButton);
  OnMarketChanged(0);
  PopulateTaskTable();
  PopulateCancelTable();
}

SimplifiedKeyBindingsDialog::~SimplifiedKeyBindingsDialog() {}

bool SimplifiedKeyBindingsDialog::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_taskBindingsTable) {
    if(event->type() == QEvent::KeyPress) {
      return FilterTaskBindingsTable(event);
    }
  } else if(object == m_ui->m_cancelsBindingsTable) {
    if(event->type() == QEvent::KeyPress) {
      return FilterCancelBindingsTable(event);
    }
  }
  return QWidget::eventFilter(object, event);
}

void SimplifiedKeyBindingsDialog::PopulateTaskTable() {
  auto selectedRow = m_ui->m_taskBindingsTable->currentRow();
  auto& orderTypes = m_orderTypes[m_currentMarket];
  while(m_ui->m_taskBindingsTable->rowCount() > 0) {
    m_ui->m_taskBindingsTable->removeRow(0);
  }
  m_taskBindings.clear();
  for(const auto& orderType : orderTypes) {
    auto rowIndex = m_ui->m_taskBindingsTable->rowCount();
    m_ui->m_taskBindingsTable->insertRow(rowIndex);
    m_ui->m_taskBindingsTable->setItem(rowIndex, ORDER_TYPE_COLUMN,
      new QTableWidgetItem(QString::fromStdString(orderType->GetText())));
    auto& availableKeyBindings = KeyBindings::TaskBinding::GetAvailableKeys();
    auto item = new QTableWidgetItem();
    item->setTextAlignment(Qt::AlignHCenter);
    m_ui->m_taskBindingsTable->setItem(rowIndex, TASK_KEY_COLUMN, item);
    for(const auto& keyBinding : availableKeyBindings) {
      const QKeySequence& keySequence = keyBinding;
      auto taskBinding = m_keyBindings.GetTaskFromBinding(m_currentMarket,
        keySequence);
      if(taskBinding.is_initialized()) {
        auto identifier = taskBinding->m_node->FindMetaData(
          KEY_BINDING_IDENTIFIER);
        if(identifier.is_initialized() &&
            orderType->GetText() == taskBinding->m_node->GetText()) {
          item->setText(keySequence.toString());
          m_taskBindings[rowIndex] = keySequence;
          break;
        }
      }
    }
  }
  m_ui->m_taskBindingsTable->setCurrentCell(selectedRow, 0);
}

void SimplifiedKeyBindingsDialog::PopulateCancelTable() {
  auto selectedRow = m_ui->m_cancelsBindingsTable->currentRow();
  while(m_ui->m_cancelsBindingsTable->rowCount() > 0) {
    m_ui->m_cancelsBindingsTable->removeRow(0);
  }
  m_cancelBindings.clear();
  for(auto i = 0; i < KeyBindings::CancelBinding::Type::COUNT; ++i) {
    auto cancelBinding = KeyBindings::CancelBinding::GetCancelBindingFromType(
      static_cast<KeyBindings::CancelBinding::Type>(i));
    auto rowIndex = m_ui->m_cancelsBindingsTable->rowCount();
    m_ui->m_cancelsBindingsTable->insertRow(rowIndex);
    m_ui->m_cancelsBindingsTable->setItem(rowIndex, CANCEL_OPTION_COLUMN,
      new QTableWidgetItem(QString::fromStdString(
      cancelBinding.m_description)));
    auto& availableKeyBindings =
      KeyBindings::CancelBinding::GetAvailableKeys();
    for(const auto& keyBinding : availableKeyBindings) {
      const QKeySequence& keySequence = keyBinding;
      auto cancelBindingProperty = m_keyBindings.GetCancelFromBinding(
        keySequence);
      if(cancelBindingProperty.is_initialized() &&
          cancelBindingProperty->m_type == cancelBinding.m_type) {
        auto item = new QTableWidgetItem(keySequence.toString());
        item->setTextAlignment(Qt::AlignHCenter);
        m_ui->m_cancelsBindingsTable->setItem(rowIndex, CANCEL_KEY_COLUMN,
          item);
        m_cancelBindings[rowIndex] = keySequence;
      }
    }
  }
  m_ui->m_cancelsBindingsTable->setCurrentCell(selectedRow, 0);
}

bool SimplifiedKeyBindingsDialog::FilterTaskBindingsTable(QEvent* event) {
  auto& keyEvent = static_cast<const QKeyEvent&>(*event);
  QKeySequence keySequence(keyEvent.modifiers() + keyEvent.key());
  auto& availableKeyBindings = KeyBindings::TaskBinding::GetAvailableKeys();
  if(keyEvent.key() == Qt::Key_Delete) {
    auto selectedRow = m_ui->m_taskBindingsTable->currentRow();
    if(selectedRow == -1) {
      return QWidget::eventFilter(m_ui->m_taskBindingsTable, event);
    }
    auto currentBindingIterator = m_taskBindings.find(selectedRow);
    if(currentBindingIterator != m_taskBindings.end()) {
      m_keyBindings.ResetTaskBinding(m_currentMarket,
        currentBindingIterator->second);
      auto item = m_ui->m_taskBindingsTable->item(selectedRow,
        TASK_KEY_COLUMN);
      item->setText(QString());
      PopulateCancelTable();
    }
    return true;
  } else if(std::find(availableKeyBindings.begin(),
      availableKeyBindings.end(), keySequence) !=
      availableKeyBindings.end()) {
    auto selectedRow = m_ui->m_taskBindingsTable->currentRow();
    if(selectedRow == -1) {
      return QWidget::eventFilter(m_ui->m_taskBindingsTable, event);
    }
    auto& baseOrderType = m_orderTypes[m_currentMarket][selectedRow];
    m_keyBindings.ResetTaskBinding(m_currentMarket, keySequence);
    auto currentBindingIterator = m_taskBindings.find(selectedRow);
    if(currentBindingIterator != m_taskBindings.end()) {
      m_keyBindings.ResetTaskBinding(m_currentMarket,
        currentBindingIterator->second);
    }
    for(auto i = 0; i < m_ui->m_taskBindingsTable->rowCount(); ++i) {
      auto item = m_ui->m_taskBindingsTable->item(i, TASK_KEY_COLUMN);
      auto previousSequence = QKeySequence::fromString(item->text());
      if(previousSequence == keySequence) {
        item->setText(QString());
      }
    }
    unique_ptr<CanvasNode> orderType;
    auto baseQuantityNode = baseOrderType->FindChild("quantity");
    if(baseQuantityNode.is_initialized()) {
      if(auto quantityNode =
          dynamic_cast<const IntegerNode*>(&*baseQuantityNode)) {
        try {
          orderType = baseOrderType->Replace("quantity",
            quantityNode->SetValue(static_cast<int>(
            m_keyBindings.GetDefaultQuantity(m_currentMarket))));
        } catch(const CanvasOperationException&) {
          orderType = CanvasNode::Clone(*baseOrderType);
        }
      } else {
        orderType = CanvasNode::Clone(*baseOrderType);
      }
    } else {
      orderType = CanvasNode::Clone(*baseOrderType);
    }
    auto taskBindingName = orderType->GetText();
    KeyBindings::TaskBinding taskBinding(taskBindingName,
      std::move(orderType));
    m_keyBindings.SetTaskBinding(m_currentMarket, keySequence, taskBinding);
    auto item = m_ui->m_taskBindingsTable->item(selectedRow, TASK_KEY_COLUMN);
    item->setText(keySequence.toString());
    PopulateCancelTable();
    return true;
  }
  return QWidget::eventFilter(m_ui->m_taskBindingsTable, event);
}

bool SimplifiedKeyBindingsDialog::FilterCancelBindingsTable(QEvent* event) {
  auto& keyEvent = static_cast<const QKeyEvent&>(*event);
  QKeySequence keySequence(keyEvent.modifiers() + keyEvent.key());
  auto& availableKeyBindings = KeyBindings::CancelBinding::GetAvailableKeys();
  if(keyEvent.key() == Qt::Key_Delete) {
    auto selectedRow = m_ui->m_cancelsBindingsTable->currentRow();
    if(selectedRow == -1) {
      return QWidget::eventFilter(m_ui->m_cancelsBindingsTable, event);
    }
    auto currentBindingIterator = m_cancelBindings.find(selectedRow);
    if(currentBindingIterator != m_cancelBindings.end()) {
      m_keyBindings.ResetCancelBinding(currentBindingIterator->second);
    }
    PopulateTaskTable();
    PopulateCancelTable();
    return true;
  } else if(std::find(availableKeyBindings.begin(),
      availableKeyBindings.end(), keySequence) !=
      availableKeyBindings.end()) {
    auto selectedRow = m_ui->m_cancelsBindingsTable->currentRow();
    if(selectedRow == -1) {
      return QWidget::eventFilter(m_ui->m_cancelsBindingsTable, event);
    }
    m_keyBindings.ResetCancelBinding(keySequence);
    auto currentBindingIterator = m_cancelBindings.find(selectedRow);
    if(currentBindingIterator != m_cancelBindings.end()) {
      m_keyBindings.ResetCancelBinding(currentBindingIterator->second);
    }
    auto cancelBinding = KeyBindings::CancelBinding::GetCancelBindingFromType(
      static_cast<KeyBindings::CancelBinding::Type>(selectedRow));
    m_keyBindings.SetCancelBinding(keySequence, cancelBinding);
    PopulateTaskTable();
    PopulateCancelTable();
    return true;
  }
  return QWidget::eventFilter(m_ui->m_cancelsBindingsTable, event);
}

void SimplifiedKeyBindingsDialog::OnClearTasksButton() {
  for(const auto& binding : m_taskBindings) {
    m_keyBindings.ResetTaskBinding(m_currentMarket, binding.second);
  }
  PopulateTaskTable();
  PopulateCancelTable();
}

void SimplifiedKeyBindingsDialog::OnClearCancelsButton() {
  for(const auto& binding : m_cancelBindings) {
    m_keyBindings.ResetCancelBinding(binding.second);
  }
  PopulateTaskTable();
  PopulateCancelTable();
}

void SimplifiedKeyBindingsDialog::OnOkButton() {
  OnApplyButton();
  Q_EMIT accept();
}

void SimplifiedKeyBindingsDialog::OnApplyButton() {
  m_userProfile->SetKeyBindings(m_keyBindings);
  m_userProfile->GetInteractionProperties() =
    m_ui->m_interactionsWidget->GetProperties();
}

void SimplifiedKeyBindingsDialog::OnMarketChanged(int index) {
  m_currentMarket = m_userProfile->GetMarketDatabase().FromDisplayName(
    m_ui->m_taskMarketComboBox->itemText(index).toStdString()).m_code;
  PopulateTaskTable();
  PopulateCancelTable();
}
