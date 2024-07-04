#include "Spire/KeyBindings/EnumAdditionalTagSchema.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/EnumBox.hpp"

using namespace boost;
using namespace Spire;

EnumAdditionalTagSchema::EnumAdditionalTagSchema(
  OrderFieldInfoTip::Model order_field_model, int key)
  : AdditionalTagSchema(std::move(order_field_model), key) {}

bool EnumAdditionalTagSchema::test(const AdditionalTag& tag) const {
  if(tag.m_key != get_key()) {
    return false;
  }
  if(tag.m_value == none) {
    return true;
  }
  auto value = get<std::string>(&*tag.m_value);
  if(!value) {
    return false;
  }
  auto& arguments = get_order_field_model().m_tag.m_arguments;
  auto i = std::find_if(arguments.begin(), arguments.end(),
    [&] (const auto& argument) {
      return argument.m_value == *value;
    });
  return i != arguments.end();
}

std::unique_ptr<CanvasNode> EnumAdditionalTagSchema::make_canvas_node(
    const optional<Nexus::Tag::Type>& value) const {
  if(!value) {
    return std::make_unique<NoneNode>(TextType::GetInstance());
  }
  auto text = get<std::string>(&*value);
  if(!text) {
    return std::make_unique<NoneNode>(TextType::GetInstance());
  }
  return std::make_unique<TextNode>(*text)->SetVisible(false);
}

AnyInputBox* EnumAdditionalTagSchema::make_input_box(
    std::shared_ptr<AdditionalTagValueModel> current) const {
  auto settings = EnumBox<QString>::Settings();
  auto cases = std::make_shared<ArrayListModel<QString>>();
  auto& arguments = get_order_field_model().m_tag.m_arguments;
  for(auto& argument : arguments) {
    cases->push(QString::fromStdString(argument.m_value));
  }
  settings.m_cases = std::move(cases);
  settings.m_current = make_transform_value_model(std::move(current),
    [] (const auto& current) {
      if(!current) {
        return QString();
      }
      return QString::fromStdString(get<std::string>(*current));
    },
    [] (const auto& current) {
      return Nexus::Tag::Type(current.toStdString());
    });
  return new AnyInputBox(*new EnumBox<QString>(std::move(settings)));
}
