#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include <array>
#include <fstream>
#include <ranges>
#include <unordered_map>
#include <Beam/Collections/Enum.hpp>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/RegionMap.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  struct LegacyInteractionsProperties {
    BEAM_ENUM(Modifier,
      PLAIN,
      SHIFT,
      ALT,
      CONTROL);
    static const auto MODIFIER_COUNT = 4;
    Quantity m_default_quantity;
    std::array<Quantity, MODIFIER_COUNT> m_quantity_increments;
    std::array<Money, MODIFIER_COUNT> m_price_increments;
    bool m_cancel_on_fill;
  };

  struct LegacyKeyBindings {
    struct TaskBinding {
      std::string m_name;
      std::shared_ptr<CanvasNode> m_node;
    };
    struct CancelBinding {
      BEAM_ENUM(Type,
        MOST_RECENT,
        MOST_RECENT_ASK,
        MOST_RECENT_BID,
        OLDEST,
        OLDEST_ASK,
        OLDEST_BID,
        ALL,
        ALL_ASKS,
        ALL_BIDS,
        CLOSEST_ASK,
        CLOSEST_BID,
        FURTHEST_ASK,
        FURTHEST_BID);
      std::string m_description;
      Type m_type;
    };
    std::unordered_map<MarketCode,
      std::unordered_map<QKeySequence, TaskBinding>> m_task_bindings;
    std::unordered_map<QKeySequence, CancelBinding> m_cancel_bindings;
    std::unordered_map<MarketCode, Quantity> m_default_quantities;
  };

  auto from_legacy(LegacyKeyBindings::CancelBinding::Type binding) {
    if(binding == LegacyKeyBindings::CancelBinding::Type::MOST_RECENT) {
      return CancelKeyBindingsModel::Operation::MOST_RECENT;
    } else if(
        binding == LegacyKeyBindings::CancelBinding::Type::MOST_RECENT_ASK) {
      return CancelKeyBindingsModel::Operation::MOST_RECENT_ASK;
    } else if(
        binding == LegacyKeyBindings::CancelBinding::Type::MOST_RECENT_BID) {
      return CancelKeyBindingsModel::Operation::MOST_RECENT_BID;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::OLDEST) {
      return CancelKeyBindingsModel::Operation::OLDEST;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::OLDEST_ASK) {
      return CancelKeyBindingsModel::Operation::OLDEST_ASK;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::OLDEST_BID) {
      return CancelKeyBindingsModel::Operation::OLDEST_BID;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::ALL) {
      return CancelKeyBindingsModel::Operation::ALL;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::ALL_ASKS) {
      return CancelKeyBindingsModel::Operation::ALL_ASKS;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::ALL_BIDS) {
      return CancelKeyBindingsModel::Operation::ALL_BIDS;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::CLOSEST_ASK) {
      return CancelKeyBindingsModel::Operation::CLOSEST_ASK;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::CLOSEST_BID) {
      return CancelKeyBindingsModel::Operation::CLOSEST_BID;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::FURTHEST_ASK) {
      return CancelKeyBindingsModel::Operation::FURTHEST_ASK;
    } else if(binding == LegacyKeyBindings::CancelBinding::Type::FURTHEST_BID) {
      return CancelKeyBindingsModel::Operation::FURTHEST_BID;
    }
    throw std::runtime_error("Invalid cancel binding.");
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<LegacyInteractionsProperties> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, LegacyInteractionsProperties& value,
        unsigned int version) {
      auto default_quantity = std::int64_t();
      shuttle.Shuttle("default_quantity", default_quantity);
      value.m_default_quantity = default_quantity;
      auto quantity_increments = std::array<
        std::int64_t, LegacyInteractionsProperties::MODIFIER_COUNT>();
      shuttle.Shuttle("quantity_increments", quantity_increments);
      for(auto i = 0; i < LegacyInteractionsProperties::MODIFIER_COUNT; ++i) {
        value.m_quantity_increments[i] = quantity_increments[i];
      }
      auto price_increments = std::array<
        std::int64_t, LegacyInteractionsProperties::MODIFIER_COUNT>();
      shuttle.Shuttle("price_increments", price_increments);
      for(auto i = 0; i < LegacyInteractionsProperties::MODIFIER_COUNT; ++i) {
        value.m_price_increments[i] = Nexus::Money(
          Nexus::Quantity(price_increments[i]) / Nexus::Quantity::MULTIPLIER);
      }
      shuttle.Shuttle("cancel_on_fill", value.m_cancel_on_fill);
    }
  };

  template<>
  struct Shuttle<LegacyKeyBindings::TaskBinding> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, LegacyKeyBindings::TaskBinding& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("node", value.m_node);
    }
  };

  template<>
  struct Shuttle<LegacyKeyBindings::CancelBinding> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, LegacyKeyBindings::CancelBinding& value,
        unsigned int version) {
      shuttle.Shuttle("description", value.m_description);
      shuttle.Shuttle("type", value.m_type);
    }
  };

  template<>
  struct Shuttle<LegacyKeyBindings> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, LegacyKeyBindings& value, unsigned int version) {
      shuttle.Shuttle("task_bindings", value.m_task_bindings);
      shuttle.Shuttle("cancel_bindings", value.m_cancel_bindings);
      shuttle.Shuttle("default_quantities", value.m_default_quantities);
    }
  };
}

namespace {
  void set_destination(CanvasNodeBuilder& builder, Destination destination) {
    builder.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(std::move(destination)));
    builder.SetVisible(SingleOrderTaskNode::DESTINATION_PROPERTY, false);
    builder.SetReadOnly(SingleOrderTaskNode::DESTINATION_PROPERTY, true);
  }

  void set_time_in_force(
      CanvasNodeBuilder& builder, TimeInForce time_in_force) {
    builder.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(std::move(time_in_force)));
    builder.SetVisible(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, false);
    builder.SetReadOnly(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY, true);
  }

  auto populate_bid_ask(CanvasNodeBuilder& builder, std::string bid_name,
      std::string ask_name, Destination destination, TimeInForce time_in_force,
      std::vector<std::unique_ptr<CanvasNode>>& nodes) {
    set_destination(builder, destination);
    set_time_in_force(builder, time_in_force);
    builder.Replace(SingleOrderTaskNode::SIDE_PROPERTY,
      std::make_unique<SideNode>(Side::BID));
    builder.SetReadOnly(SingleOrderTaskNode::SIDE_PROPERTY, true);
    builder.SetVisible(SingleOrderTaskNode::SIDE_PROPERTY, false);
    auto bid = builder.Make();
    bid = static_cast<const SingleOrderTaskNode*>(
      bid.get())->Rename(std::move(bid_name));
    nodes.push_back(std::move(bid));
    builder.Replace(SingleOrderTaskNode::SIDE_PROPERTY,
      std::make_unique<SideNode>(Side::ASK));
    builder.SetReadOnly(SingleOrderTaskNode::SIDE_PROPERTY, true);
    builder.SetVisible(SingleOrderTaskNode::SIDE_PROPERTY, false);
    auto ask = builder.Make();
    ask = static_cast<const SingleOrderTaskNode*>(
      ask.get())->Rename(std::move(ask_name));
    nodes.push_back(std::move(ask));
  }

  auto populate_bid_ask(CanvasNodeBuilder& builder, const std::string& prefix,
      Destination destination, TimeInForce time_in_force,
      std::vector<std::unique_ptr<CanvasNode>>& nodes) {
    populate_bid_ask(builder, prefix + " Bid", prefix + " Ask",
      std::move(destination), time_in_force, nodes);
  }

  auto populate_bid_ask_limit_market(CanvasNodeBuilder& builder,
      Destination destination, TimeInForce time_in_force,
      std::string limit_bid_name, std::string limit_ask_name,
      std::string market_bid_name, std::string market_ask_name,
      std::string buy_name, std::string sell_name,
      std::vector<std::unique_ptr<CanvasNode>>& nodes) {
    builder.Replace(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
      std::make_unique<OrderTypeNode>(OrderType::LIMIT));
    populate_bid_ask(builder, limit_bid_name, limit_ask_name, destination,
      time_in_force, nodes);
    builder.Replace(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
      std::make_unique<OrderTypeNode>(OrderType::MARKET));
    populate_bid_ask(builder, market_bid_name, market_ask_name, destination,
      time_in_force, nodes);
    builder.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    populate_bid_ask(
      builder, buy_name, sell_name, destination, time_in_force, nodes);
  }

  auto populate_bid_ask_limit_market(CanvasNodeBuilder& builder,
      Destination destination, TimeInForce time_in_force,
      const std::string& prefix,
      std::vector<std::unique_ptr<CanvasNode>>& nodes) {
    populate_bid_ask_limit_market(builder, std::move(destination),
      time_in_force, prefix + " Limit Bid", prefix + " Limit Ask",
      prefix + " Market Bid", prefix + " Market Ask", prefix + " Buy",
      prefix + " Sell", nodes);
  }

  auto populate_basic_order_task_nodes(Destination destination,
      std::string limit_bid_name, std::string limit_ask_name,
      std::string market_bid_name, std::string market_ask_name,
      std::string buy_name, std::string sell_name,
      std::vector<std::unique_ptr<CanvasNode>>& nodes) {
    auto limit = CanvasNodeBuilder(*GetLimitOrderTaskNode()->AddField(
      "max_floor", 111, LinkedNode::SetReferent(MaxFloorNode(), "security")));
    populate_bid_ask(limit, limit_bid_name, limit_ask_name, destination,
      TimeInForce::Type::DAY, nodes);
    auto market = CanvasNodeBuilder(*GetMarketOrderTaskNode());
    populate_bid_ask(market, market_bid_name, market_ask_name, destination,
      TimeInForce::Type::DAY, nodes);
    auto immediate = CanvasNodeBuilder(*GetMarketOrderTaskNode());
    immediate.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    populate_bid_ask(immediate, buy_name, sell_name, destination,
      TimeInForce::Type::DAY, nodes);
  }

  auto populate_basic_order_task_nodes(Destination destination,
      const std::string& prefix,
      std::vector<std::unique_ptr<CanvasNode>>& nodes) {
    populate_basic_order_task_nodes(std::move(destination),
      prefix + " Limit Bid", prefix + " Limit Ask", prefix + " Market Bid",
      prefix + " Market Ask", prefix + " Buy", prefix + " Sell", nodes);
  }

  void populate(std::vector<std::unique_ptr<CanvasNode>>& nodes,
      std::vector<std::unique_ptr<CanvasNode>> tasks) {
    nodes.insert(nodes.end(), std::make_move_iterator(tasks.begin()),
      std::make_move_iterator(tasks.end()));
  }

  auto load_legacy_interactions_properties(const std::filesystem::path& path) {
    auto interactions_properties =
      RegionMap<LegacyInteractionsProperties>("Global", {});
    try {
      auto reader =
        BasicIStreamReader<std::ifstream>(Initialize(path, std::ios::binary));
      auto buffer = SharedBuffer();
      reader.Read(Store(buffer));
      auto registry = TypeRegistry<BinarySender<SharedBuffer>>();
      RegisterSpireTypes(Store(registry));
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(registry));
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(interactions_properties);
    } catch(const std::exception&) {
      throw std::runtime_error("Unable to load interactions.");
    }
    return interactions_properties;
  }

  auto load_legacy_key_bindings(const std::filesystem::path& path) {
    auto key_bindings = LegacyKeyBindings();
    try {
      auto reader =
        BasicIStreamReader<std::ifstream>(Initialize(path, std::ios::binary));
      auto buffer = SharedBuffer();
      reader.Read(Store(buffer));
      auto registry = TypeRegistry<BinarySender<SharedBuffer>>();
      RegisterSpireTypes(Store(registry));
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(registry));
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(key_bindings);
    } catch(const std::exception&) {
      throw std::runtime_error("Unable to load key bindings.");
    }
    return key_bindings;
  }

  auto convert_legacy_key_bindings(
      const std::filesystem::path& path, MarketDatabase markets) {
    auto key_bindings_path = path / "key_bindings.dat";
    if(!std::filesystem::exists(key_bindings_path)) {
      throw std::runtime_error("key_bindings.dat not found.");
    }
    auto interactions_properties_path = path / "interactions.dat";
    if(!std::filesystem::exists(interactions_properties_path)) {
      throw std::runtime_error("interactions.dat not found.");
    }
    auto legacy_key_bindings = load_legacy_key_bindings(key_bindings_path);
    auto legacy_interactions_properties =
      load_legacy_interactions_properties(interactions_properties_path);
    auto key_bindings = std::make_shared<KeyBindingsModel>(std::move(markets));
    for(auto& task : make_default_order_task_nodes()) {
      key_bindings->get_order_task_arguments()->push(
        to_order_task_arguments(*task));
    }
    auto& arguments = *key_bindings->get_order_task_arguments();
    for(auto& tasks :
        legacy_key_bindings.m_task_bindings | std::views::values) {
      for(auto& task : tasks) {
        auto& key = task.first;
        auto& legacy_binding = task.second;
        for(auto i = 0; i != arguments.get_size(); ++i) {
          auto binding = arguments.get(i);
          if(binding.m_name == QString::fromStdString(legacy_binding.m_name)) {
            binding.m_key = key;
            arguments.set(i, binding);
          }
        }
      }
    }
    for(auto& cancel_binding : legacy_key_bindings.m_cancel_bindings) {
      key_bindings->get_cancel_key_bindings()->get_binding(
        from_legacy(cancel_binding.second.m_type))->set(cancel_binding.first);
    }
    for(auto i = legacy_interactions_properties.Begin();
        i != legacy_interactions_properties.End(); ++i) {
      auto& properties = std::get<1>(*i);
      auto interactions =
        key_bindings->get_interactions_key_bindings(std::get<0>(*i));
      interactions->get_default_quantity()->set(properties.m_default_quantity);
      interactions->get_quantity_increment(Qt::NoModifier)->set(
        properties.m_quantity_increments[
          LegacyInteractionsProperties::Modifier::PLAIN]);
      interactions->get_quantity_increment(Qt::ShiftModifier)->set(
        properties.m_quantity_increments[
          LegacyInteractionsProperties::Modifier::SHIFT]);
      interactions->get_quantity_increment(Qt::AltModifier)->set(
        properties.m_quantity_increments[
          LegacyInteractionsProperties::Modifier::ALT]);
      interactions->get_quantity_increment(Qt::ControlModifier)->set(
        properties.m_quantity_increments[
          LegacyInteractionsProperties::Modifier::CONTROL]);
      interactions->get_price_increment(Qt::NoModifier)->set(
        properties.m_price_increments[
          LegacyInteractionsProperties::Modifier::PLAIN]);
      interactions->get_price_increment(Qt::ShiftModifier)->set(
        properties.m_price_increments[
          LegacyInteractionsProperties::Modifier::SHIFT]);
      interactions->get_price_increment(Qt::AltModifier)->set(
        properties.m_price_increments[
          LegacyInteractionsProperties::Modifier::ALT]);
      interactions->get_price_increment(Qt::ControlModifier)->set(
        properties.m_price_increments[
          LegacyInteractionsProperties::Modifier::CONTROL]);
      interactions->is_cancel_on_fill()->set(properties.m_cancel_on_fill);
    }
    return key_bindings;
  }

  auto load_default_key_bindings(MarketDatabase markets) {
    auto key_bindings = std::make_shared<KeyBindingsModel>(std::move(markets));
    for(auto& task : make_default_order_task_nodes()) {
      key_bindings->get_order_task_arguments()->push(
        to_order_task_arguments(*task));
    }
    return key_bindings;
  }
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_asx_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(DefaultDestinations::ASXT(),
    "ASX TradeMatch", order_types);
  auto primary_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("R"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(Money::ZERO)));
  primary_peg.SetReadOnly("exec_inst", true);
  primary_peg.SetVisible("exec_inst", false);
  populate_bid_ask(primary_peg, "ASX TradeMatch Primary Peg",
    DefaultDestinations::ASXT(), TimeInForce::Type::DAY, order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "ASX TradeMatch Mid Peg",
    DefaultDestinations::ASXT(), TimeInForce::Type::DAY, order_types);
  auto market_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
  market_peg.SetReadOnly("exec_inst", true);
  market_peg.SetVisible("exec_inst", false);
  populate_bid_ask(market_peg, "ASX TradeMatch Market Peg",
    DefaultDestinations::ASXT(), TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_cxa_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::CXA(), "CXA", order_types);
  auto primary_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("R"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(Money::ZERO)));
  primary_peg.SetReadOnly("exec_inst", true);
  primary_peg.SetVisible("exec_inst", false);
  populate_bid_ask(primary_peg, "CXA Primary Peg", DefaultDestinations::CXA(),
    TimeInForce::Type::DAY, order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "CXA Mid Peg", DefaultDestinations::CXA(),
    TimeInForce::Type::DAY, order_types);
  auto market_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
  market_peg.SetReadOnly("exec_inst", true);
  market_peg.SetVisible("exec_inst", false);
  populate_bid_ask(market_peg, "CXA Market Peg", DefaultDestinations::CXA(),
    TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_alpha_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::ALPHA(), "Alpha", order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_chix_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::CHIX(), "CHI-X", order_types);
  auto fee_sensitive =
    CanvasNodeBuilder(*SingleOrderTaskNode().AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
        "ex_destination", 100, std::make_unique<TextNode>("SMRTFEE")));
  populate_bid_ask_limit_market(fee_sensitive, DefaultDestinations::CHIX(),
    TimeInForce::Type::DAY, "CHI-X Fee Sensitive", order_types);
  auto smart_dark =
    CanvasNodeBuilder(*SingleOrderTaskNode().AddField("max_floor", 111,
      LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
        "ex_destination", 100,
          std::make_unique<TextNode>("SMRTXDARKNR"))->AddField(
        "long_life", 7735, std::make_unique<TextNode>("Y")));
  populate_bid_ask_limit_market(smart_dark, DefaultDestinations::CHIX(),
    TimeInForce::Type::DAY, "CHI-X SMART X Dark", order_types);
  auto dark_att = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "ex_destination", 100, std::make_unique<TextNode>("CXD"))->AddField(
      "exec_inst", 18, std::make_unique<TextNode>("P")));
  dark_att.SetReadOnly("exec_inst", true);
  dark_att.SetVisible("exec_inst", false);
  populate_bid_ask(dark_att, "CHI-X Dark ATT", DefaultDestinations::CHIX(),
    TimeInForce::Type::DAY, order_types);
  auto dark_mpi = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "ex_destination", 100, std::make_unique<TextNode>("CXD"))->AddField(
      "exec_inst", 18, std::make_unique<TextNode>("x")));
  dark_mpi.SetReadOnly("exec_inst", true);
  dark_mpi.SetVisible("exec_inst", false);
  populate_bid_ask(dark_mpi, "CHI-X Dark MPI", DefaultDestinations::CHIX(),
    TimeInForce::Type::DAY, order_types);
  auto primary_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("R"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(Money::ZERO)));
  primary_peg.SetReadOnly("exec_inst", true);
  primary_peg.SetVisible("exec_inst", false);
  populate_bid_ask(primary_peg, "CHI-X Primary Peg",
    DefaultDestinations::CHIX(), TimeInForce::Type::DAY, order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "CHI-X Mid Peg", DefaultDestinations::CHIX(),
    TimeInForce::Type::DAY, order_types);
  auto dark_mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("SMRTCXD")));
  dark_mid_peg.SetReadOnly("exec_inst", true);
  dark_mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(dark_mid_peg, "CHI-X Dark Mid Peg",
    DefaultDestinations::CHIX(), TimeInForce::Type::DAY, order_types);
  auto market_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
  market_peg.SetReadOnly("exec_inst", true);
  market_peg.SetVisible("exec_inst", false);
  populate_bid_ask(market_peg, "CHI-X Market Peg", DefaultDestinations::CHIX(),
    TimeInForce::Type::DAY, order_types);
  auto multi_mid_peg =
    CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField("exec_inst", 18,
      std::make_unique<TextNode>("M"))->AddField("ex_destination", 100,
        std::make_unique<TextNode>("MULTIDARK-YCM")));
  multi_mid_peg.SetReadOnly("exec_inst", true);
  multi_mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(multi_mid_peg, "CHI-X Multi-Mid Peg",
    DefaultDestinations::CHIX(), TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_cse_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::CSE(), "CSE", order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "CSE Mid Peg", DefaultDestinations::CSE(),
    TimeInForce::Type::DAY, order_types);
  auto market_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
  market_peg.SetReadOnly("exec_inst", true);
  market_peg.SetVisible("exec_inst", false);
  populate_bid_ask(market_peg, "CSE Market Peg", DefaultDestinations::CSE(),
    TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_cse2_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::CSE2(), "CSE2", order_types);
  auto dark_limit = CanvasNodeBuilder(*GetLimitOrderTaskNode()->AddField(
    "max_floor", 111, std::make_unique<IntegerNode>(0)));
  dark_limit.SetVisible("max_floor", false);
  dark_limit.SetReadOnly("max_floor", true);
  populate_bid_ask(dark_limit, "CSE Dark Limit", DefaultDestinations::CSE2(),
    TimeInForce::Type::DAY, order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "CSE2 Mid Peg", DefaultDestinations::CSE2(),
    TimeInForce::Type::DAY, order_types);
  auto market_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
  market_peg.SetReadOnly("exec_inst", true);
  market_peg.SetVisible("exec_inst", false);
  populate_bid_ask(market_peg, "CSE2 Market Peg", DefaultDestinations::CSE2(),
    TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_cx2_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::CX2(), "CX2", order_types);
  auto primary_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("R"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(Money::ZERO)));
  primary_peg.SetReadOnly("exec_inst", true);
  primary_peg.SetVisible("exec_inst", false);
  populate_bid_ask(primary_peg, "CX2 Primary Peg", DefaultDestinations::CX2(),
    TimeInForce::Type::DAY, order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "CX2 Mid Peg", DefaultDestinations::CX2(),
    TimeInForce::Type::DAY, order_types);
  auto market_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
  market_peg.SetReadOnly("exec_inst", true);
  market_peg.SetVisible("exec_inst", false);
  populate_bid_ask(market_peg, "CX2 Market Peg", DefaultDestinations::CX2(),
    TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_lynx_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::LYNX(), "Lynx", order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_matnlp_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::MATNLP(), "MATCH Now LP", order_types);
  auto at_the_touch = CanvasNodeBuilder(*GetLimitOrderTaskNode()->AddField(
    "constraints", 6005, std::make_unique<TextNode>("PAG")));
  at_the_touch.SetVisible("constraints", false);
  at_the_touch.SetReadOnly("constraints", true);
  auto mpi = CanvasNodeBuilder(*GetLimitOrderTaskNode()->AddField(
    "constraints", 6005, std::make_unique<TextNode>("PMI")));
  mpi.SetVisible("constraints", false);
  mpi.SetReadOnly("constraints", true);
  populate_bid_ask(at_the_touch, "MATCH Now LP MPI",
    DefaultDestinations::MATNLP(), TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_matnmf_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::MATNMF(), "MATCH Now MF", order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_neoe_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::NEOE(), "NEO Lit", order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "NEOE Lit Mid Peg", DefaultDestinations::NEOE(),
    TimeInForce::Type::DAY, order_types);
  auto book = CanvasNodeBuilder(*SingleOrderTaskNode().AddField("max_floor",
    111, LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("N")));
  book.SetReadOnly("ex_destination", true);
  book.SetVisible("ex_destination", false);
  populate_bid_ask_limit_market(book, DefaultDestinations::NEOE(),
    TimeInForce::Type::DAY, "NEO Book", order_types);
  auto ioc = CanvasNodeBuilder(*GetLimitOrderTaskNode()->AddField(
    "ex_destination", 100, std::make_unique<TextNode>("N")));
  ioc.SetReadOnly("ex_destination", true);
  ioc.SetVisible("ex_destination", false);
  populate_bid_ask(ioc, "NEO Book IOC", DefaultDestinations::NEOE(),
    TimeInForce::Type::IOC, order_types);
  auto fok = CanvasNodeBuilder(*GetLimitOrderTaskNode()->AddField(
    "ex_destination", 100, std::make_unique<TextNode>("N")));
  ioc.SetReadOnly("ex_destination", true);
  ioc.SetVisible("ex_destination", false);
  populate_bid_ask(ioc, "NEO Book FOK", DefaultDestinations::NEOE(),
    TimeInForce::Type::FOK, order_types);
  auto book_mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(true)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M"))->AddField(
      "ex_destination", 100, std::make_unique<TextNode>("N")));
  book_mid_peg.SetReadOnly("exec_inst", true);
  book_mid_peg.SetVisible("exec_inst", false);
  book_mid_peg.SetReadOnly("ex_destination", true);
  book_mid_peg.SetVisible("ex_destination", false);
  populate_bid_ask(book_mid_peg, "NEOE Book Mid Peg",
    DefaultDestinations::NEOE(), TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_omega_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::OMEGA(), "Omega", order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_pure_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(
    DefaultDestinations::PURE(), "Pure", order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_tsx_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  auto limit = CanvasNodeBuilder(*GetLimitOrderTaskNode()->AddField("max_floor",
    111, LinkedNode::SetReferent(MaxFloorNode(), "security"))->AddField(
      "long_life", 7735, std::make_unique<TextNode>("Y")));
  limit.SetReadOnly("long_life", true);
  limit.SetVisible("long_life", false);
  populate_bid_ask(limit, "TSX", DefaultDestinations::TSX(),
    TimeInForce::Type::DAY, order_types);
  auto market = CanvasNodeBuilder(*GetMarketOrderTaskNode());
  populate_bid_ask(market, "TSX", DefaultDestinations::TSX(),
    TimeInForce::Type::DAY, order_types);
  auto immediate = CanvasNodeBuilder(*GetMarketOrderTaskNode());
  immediate.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
  populate_bid_ask(immediate, "TSX", DefaultDestinations::TSX(),
    TimeInForce::Type::DAY, order_types);
  auto limit_on_close = CanvasNodeBuilder(*GetLimitOrderTaskNode());
  populate_bid_ask(limit_on_close, "TSX Limit On Close",
    DefaultDestinations::TSX(), TimeInForce::Type::MOC, order_types);
  auto market_on_close = CanvasNodeBuilder(*GetMarketOrderTaskNode());
  populate_bid_ask(limit_on_close, "TSX Market On Close",
    DefaultDestinations::TSX(), TimeInForce::Type::MOC, order_types);
  auto dark_mid_point = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false));
  populate_bid_ask(dark_mid_point, "TSX Dark Mid-Point",
    DefaultDestinations::TSX(), TimeInForce::Type::DAY, order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>>
    Spire::make_default_order_task_nodes() {
  auto tasks = std::vector<std::unique_ptr<CanvasNode>>();
  populate(tasks, make_asx_order_task_nodes());
  populate(tasks, make_cxa_order_task_nodes());
  populate(tasks, make_alpha_order_task_nodes());
  populate(tasks, make_chix_order_task_nodes());
  populate(tasks, make_cse_order_task_nodes());
  populate(tasks, make_cse2_order_task_nodes());
  populate(tasks, make_cx2_order_task_nodes());
  populate(tasks, make_lynx_order_task_nodes());
  populate(tasks, make_matnlp_order_task_nodes());
  populate(tasks, make_matnmf_order_task_nodes());
  populate(tasks, make_neoe_order_task_nodes());
  populate(tasks, make_omega_order_task_nodes());
  populate(tasks, make_pure_order_task_nodes());
  populate(tasks, make_tsx_order_task_nodes());
  return tasks;
}

std::shared_ptr<KeyBindingsModel> Spire::load_key_bindings_profile(
    const std::filesystem::path& path, MarketDatabase markets) {
  auto file_path = path / "key_bindings.json";
  if(!std::filesystem::exists(file_path)) {
    auto legacy_path = path / "key_bindings.dat";
    if(std::filesystem::exists(legacy_path)) {
      return convert_legacy_key_bindings(path, std::move(markets));
    }
    return load_default_key_bindings(std::move(markets));
  }
  return std::make_shared<KeyBindingsModel>(std::move(markets));
}

void Spire::save_key_bindings_profile(
    const KeyBindingsModel& key_bindings, const std::filesystem::path& path) {
}
