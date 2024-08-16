#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const TextType& TextType::GetInstance() {
  static auto instance = std::shared_ptr<TextType>(new TextType());
  return *instance;
}

string TextType::GetName() const {
  return "Text";
}

const type_info& TextType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility TextType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void TextType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
