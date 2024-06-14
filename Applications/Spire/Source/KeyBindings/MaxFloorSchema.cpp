#include "Spire/KeyBindings/MaxFloorSchema.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

MaxFloorSchema::MaxFloorSchema()
  : AdditionalTagSchema("MaxFloor", 111) {}

std::unique_ptr<CanvasNode> MaxFloorSchema::make_canvas_node(
    const optional<Nexus::Tag::Type>& value) const {
  auto max_floor_node = [&] {
    if(value) {
      if(value->which() == Nexus::Tag::INT_INDEX) {
        return MaxFloorNode(get<int>(*value));
      } else if(value->which() == Nexus::Tag::QUANTITY_INDEX) {
        return MaxFloorNode(get<Quantity>(*value));
      } else if(value->which() == Nexus::Tag::DOUBLE_INDEX) {
        return MaxFloorNode(get<double>(*value));
      }
      throw CanvasTypeCompatibilityException();
    }
    return MaxFloorNode();
  }();
  return LinkedNode::SetReferent(std::move(max_floor_node), "security");
}
