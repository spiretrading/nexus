#include "Spire/KeyBindings/AdditionalTagSchema.hpp"

using namespace boost;
using namespace Spire;

const std::string& AdditionalTagSchema::get_name() const {
  return m_name;
}

int AdditionalTagSchema::get_key() const {
  return m_key;
}

std::unique_ptr<CanvasNode> AdditionalTagSchema::make_canvas_node() const {
  return make_canvas_node(none);
}

AdditionalTagSchema::AdditionalTagSchema(std::string name, int key)
  : m_name(std::move(name)),
    m_key(key) {}
