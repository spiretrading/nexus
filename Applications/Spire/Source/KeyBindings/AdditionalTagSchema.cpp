#include "Spire/KeyBindings/AdditionalTagSchema.hpp"

using namespace boost;
using namespace Spire;

const OrderFieldInfoTip::Model&
    AdditionalTagSchema::get_order_field_model() const {
  return m_order_field_model;
}

const std::string& AdditionalTagSchema::get_name() const {
  return m_order_field_model.m_tag.m_name;
}

int AdditionalTagSchema::get_key() const {
  return m_key;
}

std::unique_ptr<CanvasNode> AdditionalTagSchema::make_canvas_node() const {
  return make_canvas_node(none);
}

AdditionalTagSchema::AdditionalTagSchema(
    OrderFieldInfoTip::Model order_field_model, int key)
  : m_order_field_model(std::move(order_field_model)),
    m_key(key) {}
