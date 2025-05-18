#include "Spire/KeyBindings/MaxFloorSchema.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/QuantityBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto MAX_FLOOR_KEY = 111;

  auto make_order_field_info_model() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "MaxFloor";
    model.m_tag.m_description =
      "Maximum number of shares within an order to be shown on the exchange "
      "floor at any given time.";
    return model;
  }
}

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

AnyInputBox* MaxFloorSchema::make_input_box(
    std::shared_ptr<AdditionalTagValueModel> current,
    const SubmitSignal::slot_type& submission) const {
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
    style.get(ReadOnly()).
      set(border_size(0)).
      set(horizontal_padding(scale_width(8)));
  });
  quantity_box->connect_submit_signal([=] (const auto& value) {
    if(!value) {
      submission(none);
    } else {
      submission(Nexus::Tag::Type(*value));
    }
  });
  return new AnyInputBox(*quantity_box);
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

MaxFloorSchema::MaxFloorSchema()
  : AdditionalTagSchema(make_order_field_info_model(), MAX_FLOOR_KEY) {}
