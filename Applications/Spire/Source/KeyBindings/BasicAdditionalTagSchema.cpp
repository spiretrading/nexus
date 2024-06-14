#include "Spire/KeyBindings/BasicAdditionalTagSchema.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"
#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  struct visitor : static_visitor<std::unique_ptr<CanvasNode>> {
    std::unique_ptr<CanvasNode> operator ()(int value) const {
      return std::make_unique<IntegerNode>(value);
    }

    std::unique_ptr<CanvasNode> operator ()(double value) const {
      return std::make_unique<DecimalNode>(value);
    }

    std::unique_ptr<CanvasNode> operator ()(Quantity value) const {
      return std::make_unique<DecimalNode>(static_cast<double>(value));
    }

    std::unique_ptr<CanvasNode> operator ()(Money value) const {
      return std::make_unique<MoneyNode>(value);
    }

    std::unique_ptr<CanvasNode> operator ()(char value) const {
      return std::make_unique<TextNode>(std::string(1, value));
    }

    std::unique_ptr<CanvasNode> operator ()(const std::string& value) const {
      return std::make_unique<TextNode>(value);
    }

    std::unique_ptr<CanvasNode> operator ()(date value) const {
      return std::make_unique<DateTimeNode>(ptime(value, seconds(0)));
    }

    std::unique_ptr<CanvasNode> operator ()(time_duration value) const {
      return std::make_unique<DurationNode>(value);
    }

    std::unique_ptr<CanvasNode> operator ()(ptime value) const {
      return std::make_unique<DateTimeNode>(value);
    }
  };
}

BasicAdditionalTagSchema::BasicAdditionalTagSchema(
  std::string name, int key, const CanvasType& type)
  : AdditionalTagSchema(std::move(name), key),
    m_type(static_cast<std::shared_ptr<CanvasType>>(type)) {}

BasicAdditionalTagSchema::BasicAdditionalTagSchema(
  std::string name, int key, Nexus::Tag::Type default_value)
  : AdditionalTagSchema(std::move(name), key),
    m_type(static_cast<std::shared_ptr<CanvasType>>(
      apply_visitor(visitor(), default_value)->GetType())),
    m_default_value(std::move(default_value)) {}

std::unique_ptr<CanvasNode> BasicAdditionalTagSchema::make_canvas_node(
    const optional<Tag::Type>& value) const {
  if(value) {
    auto result = apply_visitor(visitor(), *value);
    if(result->GetType().GetCompatibility(*m_type) !=
        CanvasType::Compatibility::EQUAL) {
      return nullptr;
    }
    return result;
  } else if(m_default_value) {
    return make_canvas_node(*m_default_value);
  }
  return MakeDefaultCanvasNode(*m_type);
}
