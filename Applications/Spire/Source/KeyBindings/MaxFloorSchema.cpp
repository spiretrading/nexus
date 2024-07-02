#include "Spire/KeyBindings/MaxFloorSchema.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/QuantityBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

const std::shared_ptr<MaxFloorSchema>& MaxFloorSchema::get_instance() {
  static const auto schema =
    std::shared_ptr<MaxFloorSchema>(new MaxFloorSchema());
  return schema;
}

bool MaxFloorSchema::test(const AdditionalTag& tag) const {
  return tag.m_key == get_key() && (
    tag.m_value == none || tag.m_value->which() == Nexus::Tag::INT_INDEX ||
      tag.m_value->which() == Nexus::Tag::QUANTITY_INDEX ||
      tag.m_value->which() == Nexus::Tag::DOUBLE_INDEX);
}

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

AnyInputBox* MaxFloorSchema::make_input_box(
    std::shared_ptr<AdditionalTagValueModel> current) const {
  auto quantity = make_scalar_value_model_decorator(
    make_transform_value_model(std::move(current),
      [] (const auto& value) -> optional<Quantity> {
        if(!value) {
          return none;
        }
        return get<Quantity>(*value);
      },
      [] (const auto& value) -> optional<Nexus::Tag::Type> {
        if(!value) {
          return none;
        }
        return Nexus::Tag::Type(*value);
      }));
  quantity->set_minimum(Quantity(-1));
  auto quantity_box = new QuantityBox(std::move(quantity));
  update_style(*quantity_box, [] (auto& style) {
    style.get(Any()).set(border_size(0));
  });
  return new AnyInputBox(*quantity_box);
}

MaxFloorSchema::MaxFloorSchema()
  : AdditionalTagSchema("MaxFloor", 111) {}
