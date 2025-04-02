#include "Spire/KeyBindings/BasicAdditionalTagSchema.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"
#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/MoneyBox.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct ToCanvasNodeVisitor : static_visitor<std::unique_ptr<CanvasNode>> {
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

  struct ToInputBoxVisitor : CanvasTypeVisitor {
    std::shared_ptr<AdditionalTagValueModel> m_current;
    const AdditionalTagSchema::SubmitSignal::slot_type* m_submission;
    AnyInputBox* m_input_box;

    ToInputBoxVisitor(std::shared_ptr<AdditionalTagValueModel> current,
      const AdditionalTagSchema::SubmitSignal::slot_type& submission)
      : m_current(std::move(current)),
        m_submission(&submission) {}

    void Visit(const BooleanType& type) override {
    }

    void Visit(const CanvasType& type) override {
    }

    void Visit(const CurrencyType& type) override {
    }

    void Visit(const DateTimeType& type) override {
    }

    void Visit(const DecimalType& type) override {
    }

    void Visit(const DestinationType& type) override {
    }

    void Visit(const DurationType& type) override {
    }

    void Visit(const IntegerType& type) override {
    }

    void Visit(const MarketType& type) override {
    }

    void Visit(const MoneyType& type) override {
      auto box = new MoneyBox(make_scalar_value_model_decorator(
        make_transform_value_model(std::move(m_current),
          [] (const auto& value) -> optional<Money> {
            if(!value) {
              return none;
            }
            return get<Money>(*value);
          },
          [] (const auto& value) -> optional<Nexus::Tag::Type> {
            if(!value) {
              return none;
            }
            return Nexus::Tag::Type(*value);
          })));
      update_style(*box, [] (auto& style) {
        style.get(Any()).set(border_size(0));
      });
      box->connect_submit_signal(
        [submission = *m_submission] (const auto& value) {
          if(!value) {
            submission(none);
          } else {
            submission(Nexus::Tag::Type(*value));
          }
        });
      m_input_box = new AnyInputBox(*box);
    }

    void Visit(const OrderStatusType& type) override {
    }

    void Visit(const OrderTypeType& type) override {
    }

    void Visit(const SecurityType& type) override {
    }

    void Visit(const SideType& type) override {
    }

    void Visit(const TextType& type) override {
    }

    void Visit(const TimeInForceType& type) override {
    }

    void Visit(const TimeRangeType& type) override {
    }
  };
}

BasicAdditionalTagSchema::BasicAdditionalTagSchema(
  OrderFieldInfoTip::Model order_field_model, int key, const CanvasType& type)
  : AdditionalTagSchema(std::move(order_field_model), key),
    m_type(static_cast<std::shared_ptr<CanvasType>>(type)) {}

BasicAdditionalTagSchema::BasicAdditionalTagSchema(
  OrderFieldInfoTip::Model order_field_model, int key,
  Nexus::Tag::Type default_value)
  : AdditionalTagSchema(std::move(order_field_model), key),
    m_type(static_cast<std::shared_ptr<CanvasType>>(
      apply_visitor(ToCanvasNodeVisitor(), default_value)->GetType())),
    m_default_value(std::move(default_value)) {}

bool BasicAdditionalTagSchema::test(const AdditionalTag& tag) const {
  if(tag.m_key != get_key()) {
    return false;
  }
  if(tag.m_value == none) {
    return true;
  }
  auto result = apply_visitor(ToCanvasNodeVisitor(), *tag.m_value);
  return result->GetType().GetCompatibility(*m_type) ==
    CanvasType::Compatibility::EQUAL;
}

AnyInputBox* BasicAdditionalTagSchema::make_input_box(
    std::shared_ptr<AdditionalTagValueModel> current,
    const SubmitSignal::slot_type& submission) const {
  auto visitor = ToInputBoxVisitor(std::move(current), submission);
  m_type->Apply(visitor);
  return visitor.m_input_box;
}

std::unique_ptr<CanvasNode> BasicAdditionalTagSchema::make_canvas_node(
    const optional<Nexus::Tag::Type>& value) const {
  if(value) {
    auto result = apply_visitor(ToCanvasNodeVisitor(), *value);
    if(result->GetType().GetCompatibility(*m_type) !=
        CanvasType::Compatibility::EQUAL) {
      throw CanvasTypeCompatibilityException();
    }
    return result;
  } else if(m_default_value) {
    return make_canvas_node(*m_default_value);
  }
  return MakeDefaultCanvasNode(*m_type);
}
