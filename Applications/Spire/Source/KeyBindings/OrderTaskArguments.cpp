#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  template<typename T>
  typename T::Type extract(const optional<const CanvasNode&>& child) {
    if(!child) {
      return {};
    } else if(auto node = dynamic_cast<const ValueNode<T>*>(&*child)) {
      return node->GetValue();
    }
    return {};
  }

  AdditionalTag to_additional_tag(const SingleOrderTaskNode& node,
      const SingleOrderTaskNode::FieldEntry& field) {
    auto child = node.FindChild(field.m_name);
    if(!child) {
      return {};
    }
    auto value = [&] () -> Nexus::Tag::Type {
      if(field.m_type->GetNativeType() == typeid(int)) {
        return extract<IntegerType>(child);
      } else if(field.m_type->GetNativeType() == typeid(double)) {
        return extract<DecimalType>(child);
      } else if(field.m_type->GetNativeType() == typeid(Quantity)) {
        return extract<IntegerType>(child);
      } else if(field.m_type->GetNativeType() == typeid(Money)) {
        return extract<MoneyType>(child);
      } else if(field.m_type->GetNativeType() == typeid(std::string)) {
        return extract<TextType>(child);
      } else if(field.m_type->GetNativeType() == typeid(time_duration)) {
        return extract<DurationType>(child);
      } else if(field.m_type->GetNativeType() == typeid(ptime)) {
        return extract<DateTimeType>(child);
      }
      throw std::runtime_error("Unsupported tag.");
    }();
    return AdditionalTag(field.m_key, std::move(value));
  }
}

optional<const OrderTaskArguments&> Spire::find_order_task_arguments(
    const OrderTaskArgumentsListModel& arguments, const Region& region,
    const QKeySequence& key) {
  auto candidates = std::vector<int>();
  for(auto i = 0; i != arguments.get_size(); ++i) {
    auto& argument = arguments.get(i);
    if(argument.m_key == key && region <= argument.m_region) {
      candidates.push_back(i);
    }
  }
  if(candidates.empty()) {
    return none;
  }
  std::sort(candidates.begin(), candidates.end(), [&] (auto left, auto right) {
    return arguments.get(left).m_region < arguments.get(right).m_region;
  });
  return arguments.get(candidates.front());
}

std::unique_ptr<CanvasNode>
    Spire::make_canvas_node(const OrderTaskArguments& arguments,
      const AdditionalTagDatabase& additional_tags) {
  auto node = std::unique_ptr<CanvasNode>(
    std::make_unique<SingleOrderTaskNode>()->Rename(
      arguments.m_name.toStdString()));
  if(arguments.m_order_type != OrderType::NONE) {
    node = node->Replace(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
      std::make_unique<OrderTypeNode>(
        arguments.m_order_type)->SetReadOnly(true)->SetVisible(false));
    if(arguments.m_order_type == OrderType::MARKET) {
      node = node->Replace(SingleOrderTaskNode::PRICE_PROPERTY,
        node->FindChild(SingleOrderTaskNode::PRICE_PROPERTY)->
          SetVisible(false)->SetReadOnly(true));
    }
  }
  if(arguments.m_side != Side::NONE) {
    node = node->Replace(SingleOrderTaskNode::SIDE_PROPERTY,
      std::make_unique<SideNode>(
        arguments.m_side)->SetReadOnly(true)->SetVisible(false));
  }
  if(!arguments.m_destination.empty()) {
    node = node->Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(
        arguments.m_destination)->SetReadOnly(true)->SetVisible(false));
  }
  if(arguments.m_quantity == QuantitySetting::DEFAULT) {
    node = node->Replace(SingleOrderTaskNode::QUANTITY_PROPERTY,
      node->FindChild(
        SingleOrderTaskNode::QUANTITY_PROPERTY)->SetVisible(false));
  }
  if(arguments.m_time_in_force.get_type() != TimeInForce::Type::NONE) {
    node = node->Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(
        arguments.m_time_in_force)->SetReadOnly(true)->SetVisible(false));
  }
  for(auto& tag : arguments.m_additional_tags) {
    if(auto schema = Spire::find(additional_tags, arguments.m_destination,
        arguments.m_region, tag.m_key)) {
      node = static_cast<SingleOrderTaskNode*>(node.get())->AddField(
        schema->get_name(), schema->get_key(),
        schema->make_canvas_node(tag.m_value));
    }
  }
  return node;
}

OrderTaskArguments Spire::to_order_task_arguments(const CanvasNode& node) {
  auto arguments = OrderTaskArguments();
  arguments.m_name = QString::fromStdString(node.GetText());
  arguments.m_destination = extract<DestinationType>(
    node.FindChild(SingleOrderTaskNode::DESTINATION_PROPERTY));
  if(arguments.m_destination.empty()) {
    arguments.m_region = Region::make_global("Global");
  } else {
    auto& destination = DEFAULT_DESTINATIONS.from(arguments.m_destination);
    for(auto& venue : destination.m_venues) {
      arguments.m_region += venue;
    }
  }
  if(auto quantity = node.FindChild(SingleOrderTaskNode::QUANTITY_PROPERTY)) {
    if(quantity->IsVisible()) {
      arguments.m_quantity = QuantitySetting::ADJUSTABLE;
    } else {
      arguments.m_quantity = QuantitySetting::DEFAULT;
    }
  } else {
    arguments.m_quantity = QuantitySetting::ADJUSTABLE;
  }
  arguments.m_order_type = extract<OrderTypeType>(
    node.FindChild(SingleOrderTaskNode::ORDER_TYPE_PROPERTY));
  arguments.m_side =
    extract<SideType>(node.FindChild(SingleOrderTaskNode::SIDE_PROPERTY));
  arguments.m_time_in_force = extract<TimeInForceType>(
    node.FindChild(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY));
  if(auto task = dynamic_cast<const SingleOrderTaskNode*>(&node)) {
    for(auto& field : task->GetFields()) {
      arguments.m_additional_tags.push_back(to_additional_tag(*task, field));
    }
  }
  return arguments;
}

const QString& Spire::to_text(QuantitySetting setting) {
  if(setting == QuantitySetting::DEFAULT) {
    static const auto TEXT = QObject::tr("Default");
    return TEXT;
  } else if(setting == QuantitySetting::ADJUSTABLE) {
    static const auto TEXT = QObject::tr("Adjustable");
    return TEXT;
  }
  static const auto TEXT = QObject::tr("None");
  return TEXT;
}
