#include "Spire/Canvas/Types/AssetType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const AssetType& AssetType::GetInstance() {
  static auto instance = std::shared_ptr<AssetType>(new AssetType());
  return *instance;
}

string AssetType::GetName() const {
  return "Asset";
}

const type_info& AssetType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility AssetType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void AssetType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
