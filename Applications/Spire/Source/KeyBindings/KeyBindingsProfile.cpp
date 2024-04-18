#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include <array>
#include <fstream>
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

  auto populate_basic_order_task_nodes(Destination destination,
      std::string limit_bid_name, std::string limit_ask_name,
      std::string market_bid_name, std::string market_ask_name,
      std::string buy_name, std::string sell_name,
      std::vector<std::unique_ptr<CanvasNode>>& nodes) {
    auto limit = CanvasNodeBuilder(*GetLimitOrderTaskNode()->AddField(
      "max_floor", 111, LinkedNode::SetReferent(MaxFloorNode(), "security")));
    populate_bid_ask(limit, limit_bid_name, limit_ask_name, destination,
      TimeInForce(TimeInForce::Type::DAY), nodes);
    auto market = CanvasNodeBuilder(*GetMarketOrderTaskNode());
    populate_bid_ask(market, market_bid_name, market_ask_name, destination,
      TimeInForce(TimeInForce::Type::DAY), nodes);
    auto immediate = CanvasNodeBuilder(*GetMarketOrderTaskNode());
    immediate.SetVisible(SingleOrderTaskNode::QUANTITY_PROPERTY, false);
    populate_bid_ask(immediate, buy_name, sell_name, destination,
      TimeInForce(TimeInForce::Type::DAY), nodes);
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
    "ASX TradeMatch Limit Bid", "ASX TradeMatch Limit Ask",
    "ASX TradeMatch Market Bid", "ASX TradeMatch Market Ask",
    "ASX TradeMatch Buy", "ASX TradeMatch Sell", order_types);
  auto primary_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("R"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(Money::ZERO)));
  primary_peg.SetReadOnly("exec_inst", true);
  primary_peg.SetVisible("exec_inst", false);
  populate_bid_ask(primary_peg, "ASX TradeMatch Primary Peg Bid",
    "ASX TradeMatch Primary Peg Ask", DefaultDestinations::ASXT(),
    TimeInForce(TimeInForce::Type::DAY), order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "ASX TradeMatch Mid Peg Bid",
    "ASX TradeMatch Mid Peg Ask", DefaultDestinations::ASXT(),
    TimeInForce(TimeInForce::Type::DAY), order_types);
  auto market_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
  market_peg.SetReadOnly("exec_inst", true);
  market_peg.SetVisible("exec_inst", false);
  populate_bid_ask(market_peg, "ASX TradeMatch Market Peg Bid",
    "ASX TradeMatch Market Peg Ask", DefaultDestinations::ASXT(),
    TimeInForce(TimeInForce::Type::DAY), order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>> Spire::make_cxa_order_task_nodes() {
  auto order_types = std::vector<std::unique_ptr<CanvasNode>>();
  populate_basic_order_task_nodes(DefaultDestinations::CXA(),
    "CXA Limit Bid", "CXA Limit Ask", "CXA Market Bid", "CXA Market Ask",
    "CXA Buy", "CXA Sell", order_types);
  auto primary_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("R"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(Money::ZERO)));
  primary_peg.SetReadOnly("exec_inst", true);
  primary_peg.SetVisible("exec_inst", false);
  set_destination(primary_peg, DefaultDestinations::CXA());
  set_time_in_force(primary_peg, TimeInForce(TimeInForce::Type::DAY));
  populate_bid_ask(primary_peg, "CXA Primary Peg Bid", "CXA Primary Peg Ask",
    DefaultDestinations::CXA(), TimeInForce(TimeInForce::Type::DAY),
    order_types);
  auto mid_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("M")));
  mid_peg.SetReadOnly("exec_inst", true);
  mid_peg.SetVisible("exec_inst", false);
  populate_bid_ask(mid_peg, "CXA Mid Peg Bid", "CXA Mid Peg Ask",
    DefaultDestinations::CXA(), TimeInForce(TimeInForce::Type::DAY),
    order_types);
  auto market_peg = CanvasNodeBuilder(*GetPeggedOrderTaskNode(false)->AddField(
    "exec_inst", 18, std::make_unique<TextNode>("P"))->AddField(
      "peg_difference", 211, std::make_unique<MoneyNode>(-Money::CENT)));
  market_peg.SetReadOnly("exec_inst", true);
  market_peg.SetVisible("exec_inst", false);
  populate_bid_ask(market_peg, "CXA Market Peg Bid", "CXA Market Peg Ask",
    DefaultDestinations::CXA(), TimeInForce(TimeInForce::Type::DAY),
    order_types);
  return order_types;
}

std::vector<std::unique_ptr<CanvasNode>>
    Spire::make_default_order_task_nodes() {
  auto tasks = make_asx_order_task_nodes();
  auto nodes = make_cxa_order_task_nodes();
  tasks.insert(tasks.end(), std::make_move_iterator(nodes.begin()),
    std::make_move_iterator(nodes.end()));
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
