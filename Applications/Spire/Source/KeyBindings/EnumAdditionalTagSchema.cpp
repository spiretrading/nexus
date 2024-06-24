#include "Spire/KeyBindings/EnumAdditionalTagSchema.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/EnumBox.hpp"

using namespace boost;
using namespace Spire;

EnumAdditionalTagSchema::EnumAdditionalTagSchema(
  std::string name, int key, std::vector<std::string> cases)
  : AdditionalTagSchema(std::move(name), key),
    m_cases(std::move(cases)) {}

const std::vector<std::string>& EnumAdditionalTagSchema::get_cases() const {
  return m_cases;
}

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
  return std::find(m_cases.begin(), m_cases.end(), *value) != m_cases.end();
}

std::unique_ptr<CanvasNode> EnumAdditionalTagSchema::make_canvas_node(
    const optional<Nexus::Tag::Type>& value) const {
  return std::make_unique<NoneNode>(TextType::GetInstance());
}

AnyInputBox* EnumAdditionalTagSchema::make_input_box(
    std::shared_ptr<AdditionalTagValueModel> current) const {
  auto settings = EnumBox<QString>::Settings();
  auto cases = std::make_shared<ArrayListModel<QString>>();
  for(auto& c : m_cases) {
    cases->push(QString::fromStdString(c));
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
