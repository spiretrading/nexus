#include "Spire/Canvas/Types/SequenceType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const SequenceType& SequenceType::GetInstance() {
  static auto instance = std::shared_ptr<SequenceType>(new SequenceType());
  return *instance;
}

string SequenceType::GetName() const {
  return "Sequence";
}

const type_info& SequenceType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility SequenceType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void SequenceType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
