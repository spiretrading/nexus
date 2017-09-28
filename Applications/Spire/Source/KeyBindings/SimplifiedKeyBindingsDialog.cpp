#include "Spire/KeyBindings/SimplifiedKeyBindingsDialog.hpp"
#include <QKeyEvent>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderTasks/SingleOrderTask.hpp"
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
#include "Spire/Spire/UserProfile.hpp"
#include "ui_SimplifiedKeyBindingsDialog.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderTasks;
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
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "ASX TradeMatch Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "ASX TradeMatch Market Bid"));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "ASX TradeMatch Market Ask"));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename(
      "ASX TradeMatch Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename(
      "ASX TradeMatch Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ASXT()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
  }

  void PopulateCxaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "CXA Limit Bid"));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "CXA Limit Ask"));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "CXA Market Bid"));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "CXA Market Ask"));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("CXA Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("CXA Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "CXA Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "CXA Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "CXA Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "CXA Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "CXA Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "CXA Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CXA()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
  }

  void PopulateAlphaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder alphaLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "Alpha Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    alphaLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    alphaLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    alphaLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    alphaLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    alphaLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaLimitBid.Build());
    CanvasNodeBuilder alphaLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "Alpha Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    alphaLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    alphaLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    alphaLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    alphaLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    alphaLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaLimitAsk.Build());
    CanvasNodeBuilder alphaMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "Alpha Market Bid"));
    alphaMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    alphaMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    alphaMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    alphaMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    alphaMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaMarketBid.Build());
    CanvasNodeBuilder alphaMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "Alpha Market Ask"));
    alphaMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    alphaMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    alphaMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    alphaMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    alphaMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaMarketAsk.Build());
    CanvasNodeBuilder alphaBuy(*GetMarketBidOrderTaskNode()->Rename(
      "Alpha Buy"));
    alphaBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    alphaBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    alphaBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    alphaBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    alphaBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    alphaBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaBuy.Build());
    CanvasNodeBuilder alphaSell(*GetMarketAskOrderTaskNode()->Rename(
      "Alpha Sell"));
    alphaSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    alphaSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ALPHA()));
    alphaSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    alphaSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    alphaSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    alphaSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    alphaSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    alphaSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(alphaSell.Build());
  }

  void PopulateChixOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder chixLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "CHI-X Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    chixLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    chixLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    chixLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixLimitBid.Build());
    CanvasNodeBuilder chixLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "CHI-X Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    chixLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    chixLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    chixLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixLimitAsk.Build());
    CanvasNodeBuilder chixMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "CHI-X Market Bid"));
    chixMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    chixMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    chixMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMarketBid.Build());
    CanvasNodeBuilder chixMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "CHI-X Market Ask"));
    chixMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    chixMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    chixMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMarketAsk.Build());
    CanvasNodeBuilder chixBuy(
      *GetMarketBidOrderTaskNode()->Rename("CHI-X Buy"));
    chixBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    chixBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    chixBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    chixBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixBuy.Build());
    CanvasNodeBuilder chixSell(*GetMarketAskOrderTaskNode()->Rename(
      "CHI-X Sell"));
    chixSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    chixSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    chixSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    chixSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSell.Build());
    CanvasNodeBuilder chixFeeSensitiveLimitBid(
      *GetLimitBidOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixFeeSensitiveLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixFeeSensitiveLimitBid.SetVisible("ex_destination", false);
    chixFeeSensitiveLimitBid.SetReadOnly("ex_destination", true);
    chixFeeSensitiveLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixFeeSensitiveLimitBid.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixFeeSensitiveLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveLimitBid.Build());
    CanvasNodeBuilder chixFeeSensitiveLimitAsk(
      *GetLimitAskOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixFeeSensitiveLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixFeeSensitiveLimitAsk.SetVisible("ex_destination", false);
    chixFeeSensitiveLimitAsk.SetReadOnly("ex_destination", true);
    chixFeeSensitiveLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixFeeSensitiveLimitAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixFeeSensitiveLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveLimitAsk.Build());
    CanvasNodeBuilder chixFeeSensitiveMarketBid(
      *GetMarketBidOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Market Bid")->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixFeeSensitiveMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixFeeSensitiveMarketBid.SetVisible("ex_destination", false);
    chixFeeSensitiveMarketBid.SetReadOnly("ex_destination", true);
    chixFeeSensitiveMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveMarketBid.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixFeeSensitiveMarketBid.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixFeeSensitiveMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveMarketBid.Build());
    CanvasNodeBuilder chixFeeSensitiveMarketAsk(
      *GetMarketAskOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Market Ask")->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixFeeSensitiveMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixFeeSensitiveMarketAsk.SetVisible("ex_destination", false);
    chixFeeSensitiveMarketAsk.SetReadOnly("ex_destination", true);
    chixFeeSensitiveMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveMarketAsk.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixFeeSensitiveMarketAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixFeeSensitiveMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveMarketAsk.Build());
    CanvasNodeBuilder chixFeeSensitiveBuy(
      *GetMarketBidOrderTaskNode()->Rename("CHI-X Fee Sensitive Buy")->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    chixFeeSensitiveBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixFeeSensitiveBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixFeeSensitiveBuy.SetVisible("ex_destination", false);
    chixFeeSensitiveBuy.SetReadOnly("ex_destination", true);
    chixFeeSensitiveBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixFeeSensitiveBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixFeeSensitiveBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveBuy.Build());
    CanvasNodeBuilder chixFeeSensitiveSell(*GetMarketAskOrderTaskNode()->Rename(
      "CHI-X Fee Sensitive Sell")->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTFEE")));
    chixFeeSensitiveSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    chixFeeSensitiveSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixFeeSensitiveSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixFeeSensitiveSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixFeeSensitiveSell.SetVisible("ex_destination", false);
    chixFeeSensitiveSell.SetReadOnly("ex_destination", true);
    chixFeeSensitiveSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixFeeSensitiveSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixFeeSensitiveSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixFeeSensitiveSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixFeeSensitiveSell.Build());
    CanvasNodeBuilder chixSmartXDarkLimitBid(
      *GetLimitBidOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixSmartXDarkLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixSmartXDarkLimitBid.SetVisible("ex_destination", false);
    chixSmartXDarkLimitBid.SetReadOnly("ex_destination", true);
    chixSmartXDarkLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixSmartXDarkLimitBid.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixSmartXDarkLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkLimitBid.Build());
    CanvasNodeBuilder chixSmartXDarkLimitAsk(
      *GetLimitAskOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixSmartXDarkLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixSmartXDarkLimitAsk.SetVisible("ex_destination", false);
    chixSmartXDarkLimitAsk.SetReadOnly("ex_destination", true);
    chixSmartXDarkLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixSmartXDarkLimitAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixSmartXDarkLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkLimitAsk.Build());
    CanvasNodeBuilder chixSmartXDarkMarketBid(
      *GetMarketBidOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Market Bid")->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixSmartXDarkMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixSmartXDarkMarketBid.SetVisible("ex_destination", false);
    chixSmartXDarkMarketBid.SetReadOnly("ex_destination", true);
    chixSmartXDarkMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkMarketBid.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixSmartXDarkMarketBid.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixSmartXDarkMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkMarketBid.Build());
    CanvasNodeBuilder chixSmartXDarkMarketAsk(
      *GetMarketAskOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Market Ask")->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixSmartXDarkMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixSmartXDarkMarketAsk.SetVisible("ex_destination", false);
    chixSmartXDarkMarketAsk.SetReadOnly("ex_destination", true);
    chixSmartXDarkMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkMarketAsk.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixSmartXDarkMarketAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixSmartXDarkMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkMarketAsk.Build());
    CanvasNodeBuilder chixSmartXDarkBuy(
      *GetMarketBidOrderTaskNode()->Rename("CHI-X SMART X Dark Buy")->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    chixSmartXDarkBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixSmartXDarkBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixSmartXDarkBuy.SetVisible("ex_destination", false);
    chixSmartXDarkBuy.SetReadOnly("ex_destination", true);
    chixSmartXDarkBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixSmartXDarkBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixSmartXDarkBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkBuy.Build());
    CanvasNodeBuilder chixSmartXDarkSell(*GetMarketAskOrderTaskNode()->Rename(
      "CHI-X SMART X Dark Sell")->AddField(
      "ex_destination", 100, make_unique<TextNode>("SMRTXDARKNR")));
    chixSmartXDarkSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    chixSmartXDarkSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixSmartXDarkSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixSmartXDarkSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixSmartXDarkSell.SetVisible("ex_destination", false);
    chixSmartXDarkSell.SetReadOnly("ex_destination", true);
    chixSmartXDarkSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixSmartXDarkSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixSmartXDarkSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixSmartXDarkSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixSmartXDarkSell.Build());
    CanvasNodeBuilder chixPrimaryPegBid(
      *GetPeggedBidOrderTaskNode(false)->Rename(
      "CHI-X Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    chixPrimaryPegBid.SetReadOnly("exec_inst", true);
    chixPrimaryPegBid.SetVisible("exec_inst", false);
    chixPrimaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixPrimaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixPrimaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixPrimaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixPrimaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixPrimaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixPrimaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixPrimaryPegBid.Build());
    CanvasNodeBuilder chixPrimaryPegAsk(
      *GetPeggedAskOrderTaskNode(false)->Rename(
      "CHI-X Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    chixPrimaryPegAsk.SetReadOnly("exec_inst", true);
    chixPrimaryPegAsk.SetVisible("exec_inst", false);
    chixPrimaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixPrimaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixPrimaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixPrimaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixPrimaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixPrimaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixPrimaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixPrimaryPegAsk.Build());
    CanvasNodeBuilder chixMidPegBid(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CHI-X Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    chixMidPegBid.SetReadOnly("exec_inst", true);
    chixMidPegBid.SetVisible("exec_inst", false);
    chixMidPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMidPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    chixMidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    chixMidPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMidPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixMidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMidPegBid.Build());
    CanvasNodeBuilder chixMidPegAsk(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CHI-X Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    chixMidPegAsk.SetReadOnly("exec_inst", true);
    chixMidPegAsk.SetVisible("exec_inst", false);
    chixMidPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMidPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    chixMidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    chixMidPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMidPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixMidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMidPegAsk.Build());
    CanvasNodeBuilder chixDarkMidPegBid(
      *GetPeggedBidOrderTaskNode(true)->Rename(
      "CHI-X Dark Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      make_unique<TextNode>("SMRTCXD")));
    chixDarkMidPegBid.SetReadOnly("exec_inst", true);
    chixDarkMidPegBid.SetVisible("exec_inst", false);
    chixDarkMidPegBid.SetVisible("ex_destination", false);
    chixDarkMidPegBid.SetReadOnly("ex_destination", true);
    chixDarkMidPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixDarkMidPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixDarkMidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixDarkMidPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixDarkMidPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixDarkMidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixDarkMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixDarkMidPegBid.Build());
    CanvasNodeBuilder chixDarkMidPegAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename(
      "CHI-X Dark Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      make_unique<TextNode>("SMRTCXD")));
    chixDarkMidPegAsk.SetReadOnly("exec_inst", true);
    chixDarkMidPegAsk.SetVisible("exec_inst", false);
    chixDarkMidPegAsk.SetVisible("ex_destination", false);
    chixDarkMidPegAsk.SetReadOnly("ex_destination", true);
    chixDarkMidPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixDarkMidPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixDarkMidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixDarkMidPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixDarkMidPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixDarkMidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixDarkMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixDarkMidPegAsk.Build());
    CanvasNodeBuilder chixMarketPegBid(
      *GetPeggedBidOrderTaskNode(true)->Rename(
      "CHI-X Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    chixMarketPegBid.SetReadOnly("exec_inst", true);
    chixMarketPegBid.SetVisible("exec_inst", false);
    chixMarketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMarketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixMarketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixMarketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMarketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixMarketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixMarketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMarketPegBid.Build());
    CanvasNodeBuilder chixMarketPegAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename(
      "CHI-X Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    chixMarketPegAsk.SetReadOnly("exec_inst", true);
    chixMarketPegAsk.SetVisible("exec_inst", false);
    chixMarketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMarketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    chixMarketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    chixMarketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMarketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    chixMarketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    chixMarketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMarketPegAsk.Build());
    CanvasNodeBuilder chixMultiMidPegBid(
      *GetPeggedBidOrderTaskNode(true)->Rename(
      "CHI-X Multi-Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      make_unique<TextNode>("MULTIDARK-YCM")));
    chixMultiMidPegBid.SetReadOnly("exec_inst", true);
    chixMultiMidPegBid.SetVisible("exec_inst", false);
    chixMultiMidPegBid.SetReadOnly("ex_destination", true);
    chixMultiMidPegBid.SetVisible("ex_destination", false);
    chixMultiMidPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMultiMidPegBid.SetVisible(
      BaseSingleOrderTaskFactory::DESTINATION, false);
    chixMultiMidPegBid.SetReadOnly(
      BaseSingleOrderTaskFactory::DESTINATION, true);
    chixMultiMidPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMultiMidPegBid.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixMultiMidPegBid.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixMultiMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMultiMidPegBid.Build());
    CanvasNodeBuilder chixMultiMidPegAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename(
      "CHI-X Multi-Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      make_unique<TextNode>("MULTIDARK-YCM")));
    chixMultiMidPegAsk.SetReadOnly("exec_inst", true);
    chixMultiMidPegAsk.SetVisible("exec_inst", false);
    chixMultiMidPegAsk.SetReadOnly("ex_destination", true);
    chixMultiMidPegAsk.SetVisible("ex_destination", false);
    chixMultiMidPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CHIX()));
    chixMultiMidPegAsk.SetVisible(
      BaseSingleOrderTaskFactory::DESTINATION, false);
    chixMultiMidPegAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::DESTINATION, true);
    chixMultiMidPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    chixMultiMidPegAsk.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    chixMultiMidPegAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    chixMultiMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(chixMultiMidPegAsk.Build());
  }

  void PopulateCx2Orders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder cx2LimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "CX2 Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    cx2LimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2LimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2LimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2LimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2LimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    cx2LimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    cx2LimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2LimitBid.Build());
    CanvasNodeBuilder cx2LimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "CX2 Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    cx2LimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2LimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2LimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2LimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2LimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    cx2LimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    cx2LimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2LimitAsk.Build());
    CanvasNodeBuilder cx2MarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "CX2 Market Bid"));
    cx2MarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2MarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2MarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    cx2MarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    cx2MarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MarketBid.Build());
    CanvasNodeBuilder cx2MarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "CX2 Market Ask"));
    cx2MarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2MarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2MarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    cx2MarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    cx2MarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MarketAsk.Build());
    CanvasNodeBuilder cx2Buy(*GetMarketBidOrderTaskNode()->Rename("CX2 Buy"));
    cx2Buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    cx2Buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2Buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2Buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2Buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2Buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    cx2Buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    cx2Buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2Buy.Build());
    CanvasNodeBuilder cx2Sell(*GetMarketAskOrderTaskNode()->Rename(
      "CX2 Sell"));
    cx2Sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    cx2Sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2Sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2Sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2Sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2Sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    cx2Sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    cx2Sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2Sell.Build());
    CanvasNodeBuilder cx2PrimaryPegBid(
      *GetPeggedBidOrderTaskNode(true)->Rename(
      "CX2 Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    cx2PrimaryPegBid.SetReadOnly("exec_inst", true);
    cx2PrimaryPegBid.SetVisible("exec_inst", false);
    cx2PrimaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2PrimaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    cx2PrimaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    cx2PrimaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2PrimaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    cx2PrimaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    cx2PrimaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2PrimaryPegBid.Build());
    CanvasNodeBuilder cx2PrimaryPegAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename(
      "CX2 Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    cx2PrimaryPegAsk.SetReadOnly("exec_inst", true);
    cx2PrimaryPegAsk.SetVisible("exec_inst", false);
    cx2PrimaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2PrimaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    cx2PrimaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    cx2PrimaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2PrimaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    cx2PrimaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    cx2PrimaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2PrimaryPegAsk.Build());
    CanvasNodeBuilder cx2MidPegBid(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CX2 Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    cx2MidPegBid.SetReadOnly("exec_inst", true);
    cx2MidPegBid.SetVisible("exec_inst", false);
    cx2MidPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MidPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2MidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2MidPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MidPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    cx2MidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    cx2MidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MidPegBid.Build());
    CanvasNodeBuilder cx2MidPegAsk(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CX2 Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    cx2MidPegAsk.SetReadOnly("exec_inst", true);
    cx2MidPegAsk.SetVisible("exec_inst", false);
    cx2MidPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MidPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2MidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2MidPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MidPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    cx2MidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    cx2MidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MidPegAsk.Build());
    CanvasNodeBuilder cx2MarketPegBid(*GetPeggedBidOrderTaskNode(true)->Rename(
      "CX2 Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    cx2MarketPegBid.SetReadOnly("exec_inst", true);
    cx2MarketPegBid.SetVisible("exec_inst", false);
    cx2MarketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MarketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2MarketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2MarketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MarketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    cx2MarketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    cx2MarketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MarketPegBid.Build());
    CanvasNodeBuilder cx2MarketPegAsk(*GetPeggedAskOrderTaskNode(true)->Rename(
      "CX2 Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    cx2MarketPegAsk.SetReadOnly("exec_inst", true);
    cx2MarketPegAsk.SetVisible("exec_inst", false);
    cx2MarketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::CX2()));
    cx2MarketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    cx2MarketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    cx2MarketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    cx2MarketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    cx2MarketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    cx2MarketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(cx2MarketPegAsk.Build());
  }

  void PopulateLynxOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder lynxLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "Lynx Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    lynxLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    lynxLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    lynxLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    lynxLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    lynxLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxLimitBid.Build());
    CanvasNodeBuilder lynxLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "Lynx Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    lynxLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    lynxLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    lynxLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    lynxLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    lynxLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxLimitAsk.Build());
    CanvasNodeBuilder lynxMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "Lynx Market Bid"));
    lynxMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    lynxMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    lynxMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    lynxMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    lynxMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxMarketBid.Build());
    CanvasNodeBuilder lynxMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "Lynx Market Ask"));
    lynxMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    lynxMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    lynxMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    lynxMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    lynxMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxMarketAsk.Build());
    CanvasNodeBuilder lynxBuy(*GetMarketBidOrderTaskNode()->Rename(
      "Lynx Buy"));
    lynxBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    lynxBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    lynxBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    lynxBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    lynxBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    lynxBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxBuy.Build());
    CanvasNodeBuilder lynxSell(*GetMarketAskOrderTaskNode()->Rename(
      "Lynx Sell"));
    lynxSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    lynxSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::LYNX()));
    lynxSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    lynxSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    lynxSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    lynxSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    lynxSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    lynxSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(lynxSell.Build());
  }

  void PopulateMatchNowLpOrders(
      vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder matnlpLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "MATCH Now LP Limit Bid"));
    matnlpLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    matnlpLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    matnlpLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpLimitBid.Build());
    CanvasNodeBuilder matnlpLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "MATCH Now LP Limit Ask"));
    matnlpLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    matnlpLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    matnlpLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpLimitAsk.Build());
    CanvasNodeBuilder matnlpAttBid(*GetLimitBidOrderTaskNode()->Rename(
      "MATCH Now LP At-The-Touch Bid")->AddField(
      "constraints", 6005, make_unique<TextNode>("PAG")));
    matnlpAttBid.SetVisible("constraints", false);
    matnlpAttBid.SetReadOnly("constraints", true);
    matnlpAttBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpAttBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpAttBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpAttBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpAttBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnlpAttBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnlpAttBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpAttBid.Build());
    CanvasNodeBuilder matnlpAttAsk(*GetLimitAskOrderTaskNode()->Rename(
      "MATCH Now LP At-The-Touch Ask")->AddField(
      "constraints", 6005, make_unique<TextNode>("PAG")));
    matnlpAttAsk.SetVisible("constraints", false);
    matnlpAttAsk.SetReadOnly("constraints", true);
    matnlpAttAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpAttAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpAttAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpAttAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpAttAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnlpAttAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnlpAttAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpAttAsk.Build());
    CanvasNodeBuilder matnlpMpiBid(*GetLimitBidOrderTaskNode()->Rename(
      "MATCH Now LP MPI Bid")->AddField(
      "constraints", 6005, make_unique<TextNode>("PMI")));
    matnlpMpiBid.SetVisible("constraints", false);
    matnlpMpiBid.SetReadOnly("constraints", true);
    matnlpMpiBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpMpiBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpMpiBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpMpiBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpMpiBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnlpMpiBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnlpMpiBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpMpiBid.Build());
    CanvasNodeBuilder matnlpMpiAsk(*GetLimitAskOrderTaskNode()->Rename(
      "MATCH Now LP MPI Ask")->AddField(
      "constraints", 6005, make_unique<TextNode>("PMI")));
    matnlpMpiAsk.SetVisible("constraints", false);
    matnlpMpiAsk.SetReadOnly("constraints", true);
    matnlpMpiAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpMpiAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpMpiAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpMpiAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpMpiAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnlpMpiAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnlpMpiAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpMpiAsk.Build());
    CanvasNodeBuilder matnlpMarketBid(
      *GetMarketBidOrderTaskNode()->Rename("MATCH Now LP Market Bid"));
    matnlpMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    matnlpMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    matnlpMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpMarketBid.Build());
    CanvasNodeBuilder matnlpMarketAsk(
      *GetMarketAskOrderTaskNode()->Rename("MATCH Now LP Market Ask"));
    matnlpMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    matnlpMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    matnlpMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpMarketAsk.Build());
    CanvasNodeBuilder matnlpBuy(*GetMarketBidOrderTaskNode()->Rename(
      "MATCH Now LP Buy"));
    matnlpBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    matnlpBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnlpBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnlpBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpBuy.Build());
    CanvasNodeBuilder matnlpSell(
      *GetMarketAskOrderTaskNode()->Rename("MATCH Now LP Sell"));
    matnlpSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    matnlpSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNLP()));
    matnlpSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnlpSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnlpSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnlpSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnlpSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnlpSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnlpSell.Build());
  }

  void PopulateMatchNowMfOrders(
      vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder matnMarketFlowLimitBid(
      *GetLimitBidOrderTaskNode()->Rename("MATCH Now MF Limit Bid"));
    matnMarketFlowLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnMarketFlowLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    matnMarketFlowLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    matnMarketFlowLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnMarketFlowLimitBid.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnMarketFlowLimitBid.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnMarketFlowLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnMarketFlowLimitBid.Build());
    CanvasNodeBuilder matnMarketFlowLimitAsk(
      *GetLimitAskOrderTaskNode()->Rename("MATCH Now MF Limit Ask"));
    matnMarketFlowLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnMarketFlowLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    matnMarketFlowLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    matnMarketFlowLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnMarketFlowLimitAsk.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnMarketFlowLimitAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnMarketFlowLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnMarketFlowLimitAsk.Build());
    CanvasNodeBuilder matnMarketFlowMarketBid(
      *GetMarketBidOrderTaskNode()->Rename("MATCH Now MF Market Bid"));
    matnMarketFlowMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnMarketFlowMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    matnMarketFlowMarketBid.SetReadOnly(
      BaseSingleOrderTaskFactory::DESTINATION, true);
    matnMarketFlowMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnMarketFlowMarketBid.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnMarketFlowMarketBid.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnMarketFlowMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnMarketFlowMarketBid.Build());
    CanvasNodeBuilder matnMarketFlowMarketAsk(
      *GetMarketAskOrderTaskNode()->Rename("MATCH Now MF Market Ask"));
    matnMarketFlowMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnMarketFlowMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    matnMarketFlowMarketAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::DESTINATION, true);
    matnMarketFlowMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnMarketFlowMarketAsk.SetVisible(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnMarketFlowMarketAsk.SetReadOnly(
      BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnMarketFlowMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnMarketFlowMarketAsk.Build());
    CanvasNodeBuilder matnmfBuy(*GetMarketBidOrderTaskNode()->Rename(
      "MATCH Now MF Buy"));
    matnmfBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    matnmfBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnmfBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnmfBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnmfBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnmfBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnmfBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnmfBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnmfBuy.Build());
    CanvasNodeBuilder matnmfSell(
      *GetMarketAskOrderTaskNode()->Rename("MATCH Now MF Sell"));
    matnmfSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    matnmfSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::MATNMF()));
    matnmfSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    matnmfSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    matnmfSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    matnmfSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    matnmfSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    matnmfSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(matnmfSell.Build());
  }

  void PopulateNeoeOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder litLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "NEO Lit Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    litLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    litLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    litLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    litLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    litLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litLimitBid.Build());
    CanvasNodeBuilder litLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NEO Lit Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    litLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    litLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    litLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    litLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    litLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litLimitAsk.Build());
    CanvasNodeBuilder litMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "NEO Lit Market Bid"));
    litMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    litMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    litMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    litMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    litMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litMarketBid.Build());
    CanvasNodeBuilder litMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NEO Lit Market Ask"));
    litMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    litMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    litMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    litMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    litMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litMarketAsk.Build());
    CanvasNodeBuilder litBuy(*GetMarketBidOrderTaskNode()->Rename(
      "NEO Lit Buy"));
    litBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    litBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    litBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    litBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    litBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    litBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litBuy.Build());
    CanvasNodeBuilder litSell(*GetMarketAskOrderTaskNode()->Rename(
      "NEO Lit Sell"));
    litSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    litSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    litSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    litSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    litSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    litSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litSell.Build());
    CanvasNodeBuilder litMidPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NEOE Lit Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    litMidPegBid.SetReadOnly("exec_inst", true);
    litMidPegBid.SetVisible("exec_inst", false);
    litMidPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litMidPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    litMidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    litMidPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litMidPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    litMidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    litMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litMidPegBid.Build());
    CanvasNodeBuilder litMidPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NEOE Lit Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    litMidPegAsk.SetReadOnly("exec_inst", true);
    litMidPegAsk.SetVisible("exec_inst", false);
    litMidPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    litMidPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    litMidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    litMidPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    litMidPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    litMidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    litMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(litMidPegAsk.Build());
    CanvasNodeBuilder neoLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "NEO Book Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, make_unique<TextNode>("N")));
    neoLimitBid.SetReadOnly("ex_destination", true);
    neoLimitBid.SetVisible("ex_destination", false);
    neoLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    neoLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    neoLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    neoLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    neoLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoLimitBid.Build());
    CanvasNodeBuilder neoLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NEO Book Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, make_unique<TextNode>("N")));
    neoLimitAsk.SetReadOnly("ex_destination", true);
    neoLimitAsk.SetVisible("ex_destination", false);
    neoLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    neoLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    neoLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    neoLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    neoLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoLimitAsk.Build());
    CanvasNodeBuilder neoMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "NEO Book Market Bid")->AddField("ex_destination", 100,
      make_unique<TextNode>("N")));
    neoMarketBid.SetReadOnly("ex_destination", true);
    neoMarketBid.SetVisible("ex_destination", false);
    neoMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    neoMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    neoMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    neoMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    neoMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoMarketBid.Build());
    CanvasNodeBuilder neoMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NEO Book Market Ask")->AddField("ex_destination", 100,
      make_unique<TextNode>("N")));
    neoMarketAsk.SetReadOnly("ex_destination", true);
    neoMarketAsk.SetVisible("ex_destination", false);
    neoMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    neoMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    neoMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    neoMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    neoMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoMarketAsk.Build());
    CanvasNodeBuilder neoBuy(*GetMarketBidOrderTaskNode()->Rename(
      "NEO Book Buy")->AddField("ex_destination", 100,
      make_unique<TextNode>("N")));
    neoBuy.SetReadOnly("ex_destination", true);
    neoBuy.SetVisible("ex_destination", false);
    neoBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    neoBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    neoBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    neoBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    neoBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    neoBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoBuy.Build());
    CanvasNodeBuilder neoSell(*GetMarketAskOrderTaskNode()->Rename(
      "NEO Book Sell")->AddField("ex_destination", 100,
      make_unique<TextNode>("N")));
    neoSell.SetReadOnly("ex_destination", true);
    neoSell.SetVisible("ex_destination", false);
    neoSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    neoSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    neoSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    neoSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    neoSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    neoSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoSell.Build());
    CanvasNodeBuilder neoMidPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NEOE Book Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      make_unique<TextNode>("N")));
    neoMidPegBid.SetReadOnly("ex_destination", true);
    neoMidPegBid.SetVisible("ex_destination", false);
    neoMidPegBid.SetReadOnly("exec_inst", true);
    neoMidPegBid.SetVisible("exec_inst", false);
    neoMidPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoMidPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    neoMidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    neoMidPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoMidPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    neoMidPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    neoMidPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoMidPegBid.Build());
    CanvasNodeBuilder neoMidPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NEOE Book Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M"))->AddField("ex_destination", 100,
      make_unique<TextNode>("N")));
    neoMidPegAsk.SetReadOnly("ex_destination", true);
    neoMidPegAsk.SetVisible("ex_destination", false);
    neoMidPegAsk.SetReadOnly("exec_inst", true);
    neoMidPegAsk.SetVisible("exec_inst", false);
    neoMidPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NEOE()));
    neoMidPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    neoMidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    neoMidPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    neoMidPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    neoMidPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    neoMidPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(neoMidPegAsk.Build());
  }

  void PopulateOmegaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder omegaLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "Omega Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    omegaLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    omegaLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    omegaLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    omegaLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    omegaLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaLimitBid.Build());
    CanvasNodeBuilder omegaLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "Omega Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    omegaLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    omegaLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    omegaLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    omegaLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    omegaLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaLimitAsk.Build());
    CanvasNodeBuilder omegaMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "Omega Market Bid"));
    omegaMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    omegaMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    omegaMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    omegaMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    omegaMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaMarketBid.Build());
    CanvasNodeBuilder omegaMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "Omega Market Ask"));
    omegaMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    omegaMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    omegaMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    omegaMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    omegaMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaMarketAsk.Build());
    CanvasNodeBuilder omegaBuy(*GetMarketBidOrderTaskNode()->Rename(
      "Omega Buy"));
    omegaBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    omegaBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    omegaBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    omegaBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    omegaBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    omegaBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaBuy.Build());
    CanvasNodeBuilder omegaSell(*GetMarketAskOrderTaskNode()->Rename(
      "Omega Sell"));
    omegaSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    omegaSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::OMEGA()));
    omegaSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    omegaSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    omegaSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    omegaSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    omegaSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    omegaSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(omegaSell.Build());
  }

  void PopulatePureOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder pureLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "Pure Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    pureLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    pureLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    pureLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    pureLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    pureLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureLimitBid.Build());
    CanvasNodeBuilder pureLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "Pure Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    pureLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    pureLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    pureLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    pureLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    pureLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureLimitAsk.Build());
    CanvasNodeBuilder pureMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "Pure Market Bid"));
    pureMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    pureMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    pureMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    pureMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    pureMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureMarketBid.Build());
    CanvasNodeBuilder pureMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "Pure Market Ask"));
    pureMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    pureMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    pureMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    pureMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    pureMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureMarketAsk.Build());
    CanvasNodeBuilder pureBuy(*GetMarketBidOrderTaskNode()->Rename(
      "Pure Buy"));
    pureBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    pureBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    pureBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    pureBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    pureBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    pureBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureBuy.Build());
    CanvasNodeBuilder pureSell(*GetMarketAskOrderTaskNode()->Rename(
      "Pure Sell"));
    pureSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    pureSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::PURE()));
    pureSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    pureSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    pureSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    pureSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    pureSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    pureSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(pureSell.Build());
  }

  void PopulateTsxOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder tsxLimitBid(*GetLimitBidOrderTaskNode()->Rename(
      "TSX Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    tsxLimitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxLimitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    tsxLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    tsxLimitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxLimitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    tsxLimitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    tsxLimitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxLimitBid.Build());
    CanvasNodeBuilder tsxLimitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "TSX Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    tsxLimitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxLimitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    tsxLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    tsxLimitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxLimitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    tsxLimitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    tsxLimitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxLimitAsk.Build());
    CanvasNodeBuilder tsxMarketBid(*GetMarketBidOrderTaskNode()->Rename(
      "TSX Market Bid"));
    tsxMarketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxMarketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    tsxMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    tsxMarketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxMarketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    tsxMarketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    tsxMarketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxMarketBid.Build());
    CanvasNodeBuilder tsxMarketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "TSX Market Ask"));
    tsxMarketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxMarketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    tsxMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    tsxMarketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxMarketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    tsxMarketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    tsxMarketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxMarketAsk.Build());
    CanvasNodeBuilder tsxBuy(*GetMarketBidOrderTaskNode()->Rename("TSX Buy"));
    tsxBuy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    tsxBuy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxBuy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    tsxBuy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    tsxBuy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxBuy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    tsxBuy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    tsxBuy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxBuy.Build());
    CanvasNodeBuilder tsxSell(*GetMarketAskOrderTaskNode()->Rename(
      "TSX Sell"));
    tsxSell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    tsxSell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxSell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    tsxSell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    tsxSell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxSell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    tsxSell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    tsxSell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxSell.Build());
    CanvasNodeBuilder tsxLimitOnCloseBid(*GetLimitBidOrderTaskNode()->Rename(
      "TSX Limit On Close Bid"));
    tsxLimitOnCloseBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxLimitOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    tsxLimitOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    tsxLimitOnCloseBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    tsxLimitOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    tsxLimitOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    tsxLimitOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxLimitOnCloseBid.Build());
    CanvasNodeBuilder tsxLimitOnCloseAsk(*GetLimitAskOrderTaskNode()->Rename(
      "TSX Limit On Close Ask"));
    tsxLimitOnCloseAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxLimitOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    tsxLimitOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    tsxLimitOnCloseAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    tsxLimitOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    tsxLimitOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    tsxLimitOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxLimitOnCloseAsk.Build());
    CanvasNodeBuilder tsxMarketOnCloseBid(*GetMarketBidOrderTaskNode()->Rename(
      "TSX Market On Close Bid"));
    tsxMarketOnCloseBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxMarketOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    tsxMarketOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    tsxMarketOnCloseBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    tsxMarketOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    tsxMarketOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    tsxMarketOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxMarketOnCloseBid.Build());
    CanvasNodeBuilder tsxMarketOnCloseAsk(*GetMarketAskOrderTaskNode()->Rename(
      "TSX Market On Close Ask"));
    tsxMarketOnCloseAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxMarketOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    tsxMarketOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    tsxMarketOnCloseAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    tsxMarketOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    tsxMarketOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    tsxMarketOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxMarketOnCloseAsk.Build());
    CanvasNodeBuilder tsxDarkMidPointBid(
      *GetPeggedBidOrderTaskNode(true)->Rename("TSX Dark Mid-Point Bid"));
    tsxDarkMidPointBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxDarkMidPointBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    tsxDarkMidPointBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    tsxDarkMidPointBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxDarkMidPointBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    tsxDarkMidPointBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    tsxDarkMidPointBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxDarkMidPointBid.Build());
    CanvasNodeBuilder tsxDarkMidPointAsk(
      *GetPeggedAskOrderTaskNode(true)->Rename("TSX Dark Mid-Point Ask"));
    tsxDarkMidPointAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::TSX()));
    tsxDarkMidPointAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    tsxDarkMidPointAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    tsxDarkMidPointAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    tsxDarkMidPointAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    tsxDarkMidPointAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    tsxDarkMidPointAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(tsxDarkMidPointAsk.Build());
  }

  void PopulateAmexOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "AMEX Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "AMEX Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "AMEX Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "AMEX Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("AMEX Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("AMEX Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "AMEX Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "AMEX Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "AMEX Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "AMEX Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "AMEX Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "AMEX Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::AMEX()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
  }

  void PopulateArcaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "ARCA Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "ARCA Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "ARCA Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "ARCA Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("ARCA Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("ARCA Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ARCA Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ARCA Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ARCA Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ARCA Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "ARCA Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "ARCA Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::ARCA()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
  }

  void PopulateBatsOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "BATS Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "BATS Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "BATS Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "BATS Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("BATS Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("BATS Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATS Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATS Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATS Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATS Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATS Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATS Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATS()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
  }

  void PopulateBatyOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "BATY Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "BATY Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "BATY Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "BATY Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("BATY Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("BATY Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATY Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATY Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATY Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATY Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "BATY Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "BATY Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::BATY()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
  }

  void PopulateEdgaOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "EDGA Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "EDGA Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "EDGA Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "EDGA Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("EDGA Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("EDGA Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGA Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGA Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGA Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGA Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGA Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGA Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGA()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
  }

  void PopulateEdgxOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "EDGX Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "EDGX Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "EDGX Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "EDGX Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("EDGX Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("EDGX Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGX Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGX Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGX Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGX Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "EDGX Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "EDGX Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::EDGX()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
  }

  void PopulateNyseOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "NYSE Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NYSE Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "NYSE Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NYSE Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("NYSE Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename("NYSE Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NYSE Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NYSE Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NYSE Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NYSE Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NYSE Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NYSE Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
  }

  void PopulateNyseOpenCloseOrders(
      vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitOnOpenBid(*GetLimitBidOrderTaskNode()->Rename(
      "NYSE Limit On Open Bid"));
    limitOnOpenBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitOnOpenBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitOnOpenBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitOnOpenBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    limitOnOpenBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    limitOnOpenBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    limitOnOpenBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnOpenBid.Build());
    CanvasNodeBuilder limitOnOpenAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NYSE Limit On Open Ask"));
    limitOnOpenAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitOnOpenAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitOnOpenAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitOnOpenAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    limitOnOpenAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    limitOnOpenAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    limitOnOpenAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnOpenAsk.Build());
    CanvasNodeBuilder marketOnOpenBid(*GetMarketBidOrderTaskNode()->Rename(
      "NYSE Market On Open Bid"));
    marketOnOpenBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketOnOpenBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketOnOpenBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketOnOpenBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    marketOnOpenBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketOnOpenBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketOnOpenBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnOpenBid.Build());
    CanvasNodeBuilder marketOnOpenAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NYSE Market On Open Ask"));
    marketOnOpenAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketOnOpenAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketOnOpenAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketOnOpenAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    marketOnOpenAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketOnOpenAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketOnOpenAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnOpenAsk.Build());
    CanvasNodeBuilder limitOnCloseBid(*GetLimitBidOrderTaskNode()->Rename(
      "NYSE Limit On Close Bid"));
    limitOnCloseBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitOnCloseBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    limitOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    limitOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    limitOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnCloseBid.Build());
    CanvasNodeBuilder limitOnCloseAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NYSE Limit On Close Ask"));
    limitOnCloseAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    limitOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitOnCloseAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    limitOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    limitOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    limitOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnCloseAsk.Build());
    CanvasNodeBuilder marketOnCloseBid(*GetMarketBidOrderTaskNode()->Rename(
      "NYSE Market On Close Bid"));
    marketOnCloseBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    marketOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    marketOnCloseBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    marketOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnCloseBid.Build());
    CanvasNodeBuilder marketOnCloseAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NYSE Market On Close Ask"));
    marketOnCloseAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NYSE()));
    marketOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    marketOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    marketOnCloseAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    marketOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnCloseAsk.Build());
  }

  void PopulateNasdaqOrders(vector<unique_ptr<const CanvasNode>>& orderTypes) {
    CanvasNodeBuilder limitBid(*GetLimitBidOrderTaskNode()->Rename(
      "NASDAQ Limit Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitBid.Build());
    CanvasNodeBuilder limitAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NASDAQ Limit Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    limitAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    limitAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    limitAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    limitAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitAsk.Build());
    CanvasNodeBuilder marketBid(*GetMarketBidOrderTaskNode()->Rename(
      "NASDAQ Market Bid")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketBid.Build());
    CanvasNodeBuilder marketAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NASDAQ Market Ask")->AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security")));
    marketAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    marketAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    marketAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketAsk.Build());
    CanvasNodeBuilder buy(*GetMarketBidOrderTaskNode()->Rename("NASDAQ Buy"));
    buy.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    buy.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    buy.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    buy.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    buy.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    buy.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    buy.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(buy.Build());
    CanvasNodeBuilder sell(*GetMarketAskOrderTaskNode()->Rename(
      "NASDAQ Sell"));
    sell.SetVisible(BaseSingleOrderTaskFactory::QUANTITY, false);
    sell.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    sell.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    sell.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    sell.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    sell.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    sell.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(sell.Build());
    CanvasNodeBuilder primaryPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NASDAQ Primary Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegBid.SetReadOnly("exec_inst", true);
    primaryPegBid.SetVisible("exec_inst", false);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegBid.Build());
    CanvasNodeBuilder primaryPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NASDAQ Primary Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("R"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(Money::ZERO)));
    primaryPegAsk.SetReadOnly("exec_inst", true);
    primaryPegAsk.SetVisible("exec_inst", false);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    primaryPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    primaryPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    primaryPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    primaryPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(primaryPegAsk.Build());
    CanvasNodeBuilder midPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NASDAQ Mid Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegBid.SetReadOnly("exec_inst", true);
    midPegBid.SetVisible("exec_inst", false);
    midPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegBid.Build());
    CanvasNodeBuilder midPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NASDAQ Mid Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("M")));
    midPegAsk.SetReadOnly("exec_inst", true);
    midPegAsk.SetVisible("exec_inst", false);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    midPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    midPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE, false);
    midPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE, true);
    midPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(midPegAsk.Build());
    CanvasNodeBuilder marketPegBid(*GetPeggedBidOrderTaskNode(false)->Rename(
      "NASDAQ Market Peg Bid")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegBid.SetReadOnly("exec_inst", true);
    marketPegBid.SetVisible("exec_inst", false);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegBid.Build());
    CanvasNodeBuilder marketPegAsk(*GetPeggedAskOrderTaskNode(false)->Rename(
      "NASDAQ Market Peg Ask")->AddField("exec_inst", 18,
      make_unique<TextNode>("P"))->AddField("peg_difference", 211,
      make_unique<MoneyNode>(-Money::CENT)));
    marketPegAsk.SetReadOnly("exec_inst", true);
    marketPegAsk.SetVisible("exec_inst", false);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketPegAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::DAY)));
    marketPegAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketPegAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketPegAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketPegAsk.Build());
    CanvasNodeBuilder limitOnOpenBid(*GetLimitBidOrderTaskNode()->Rename(
      "NASDAQ Limit On Open Bid"));
    limitOnOpenBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitOnOpenBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitOnOpenBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitOnOpenBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    limitOnOpenBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    limitOnOpenBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    limitOnOpenBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnOpenBid.Build());
    CanvasNodeBuilder limitOnOpenAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NASDAQ Limit On Open Ask"));
    limitOnOpenAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitOnOpenAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitOnOpenAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitOnOpenAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    limitOnOpenAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    limitOnOpenAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    limitOnOpenAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnOpenAsk.Build());
    CanvasNodeBuilder marketOnOpenBid(*GetMarketBidOrderTaskNode()->Rename(
      "NASDAQ Market On Open Bid"));
    marketOnOpenBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketOnOpenBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketOnOpenBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketOnOpenBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    marketOnOpenBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketOnOpenBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketOnOpenBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnOpenBid.Build());
    CanvasNodeBuilder marketOnOpenAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NASDAQ Market On Open Ask"));
    marketOnOpenAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketOnOpenAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    marketOnOpenAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    marketOnOpenAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::OPG)));
    marketOnOpenAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketOnOpenAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketOnOpenAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnOpenAsk.Build());
    CanvasNodeBuilder limitOnCloseBid(*GetLimitBidOrderTaskNode()->Rename(
      "NASDAQ Limit On Close Bid"));
    limitOnCloseBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitOnCloseBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    limitOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    limitOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    limitOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnCloseBid.Build());
    CanvasNodeBuilder limitOnCloseAsk(*GetLimitAskOrderTaskNode()->Rename(
      "NASDAQ Limit On Close Ask"));
    limitOnCloseAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    limitOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION, false);
    limitOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION, true);
    limitOnCloseAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    limitOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    limitOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    limitOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(limitOnCloseAsk.Build());
    CanvasNodeBuilder marketOnCloseBid(*GetMarketBidOrderTaskNode()->Rename(
      "NASDAQ Market On Close Bid"));
    marketOnCloseBid.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    marketOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    marketOnCloseBid.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    marketOnCloseBid.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketOnCloseBid.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketOnCloseBid.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnCloseBid.Build());
    CanvasNodeBuilder marketOnCloseAsk(*GetMarketAskOrderTaskNode()->Rename(
      "NASDAQ Market On Close Ask"));
    marketOnCloseAsk.Replace(BaseSingleOrderTaskFactory::DESTINATION,
      make_unique<DestinationNode>(DefaultDestinations::NASDAQ()));
    marketOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::DESTINATION,
      false);
    marketOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::DESTINATION,
      true);
    marketOnCloseAsk.Replace(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      make_unique<TimeInForceNode>(TimeInForce(TimeInForce::Type::MOC)));
    marketOnCloseAsk.SetVisible(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      false);
    marketOnCloseAsk.SetReadOnly(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
      true);
    marketOnCloseAsk.SetMetaData("", KEY_BINDING_IDENTIFIER, true);
    orderTypes.emplace_back(marketOnCloseAsk.Build());
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
    auto& nasdaqOrderTypes = orderTypes[DefaultMarkets::NASDAQ()];
    PopulateArcaOrders(nasdaqOrderTypes);
    PopulateBatsOrders(nasdaqOrderTypes);
    PopulateBatyOrders(nasdaqOrderTypes);
    PopulateEdgaOrders(nasdaqOrderTypes);
    PopulateEdgxOrders(nasdaqOrderTypes);
    PopulateNasdaqOrders(nasdaqOrderTypes);
    auto& nyseOrderTypes = orderTypes[DefaultMarkets::NYSE()];
    PopulateArcaOrders(nyseOrderTypes);
    PopulateBatsOrders(nyseOrderTypes);
    PopulateBatyOrders(nyseOrderTypes);
    PopulateEdgaOrders(nyseOrderTypes);
    PopulateEdgxOrders(nyseOrderTypes);
    PopulateNyseOrders(nyseOrderTypes);
    PopulateNyseOpenCloseOrders(nyseOrderTypes);
    PopulateNasdaqOrders(nyseOrderTypes);
    auto& tsxOrderTypes = orderTypes[DefaultMarkets::TSX()];
    PopulateAlphaOrders(tsxOrderTypes);
    PopulateChixOrders(tsxOrderTypes);
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
    RefType<UserProfile> userProfile, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_SimplifiedKeyBindingsDialog>()),
      m_userProfile(userProfile.Get()),
      m_keyBindings(m_userProfile->GetKeyBindings()) {
  m_ui->setupUi(this);
  m_ui->m_interactionsWidget->Initialize(Ref(*m_userProfile));
  m_orderTypes = SetupOrderTypes();
  for(const auto& market : m_userProfile->GetMarketDatabase().GetEntries()) {
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
            quantityNode->SetValue(m_keyBindings.GetDefaultQuantity(
            m_currentMarket)));
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
