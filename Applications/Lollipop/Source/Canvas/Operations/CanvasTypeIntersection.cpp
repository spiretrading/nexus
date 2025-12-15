#include "Spire/Canvas/Operations/CanvasTypeIntersection.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

std::shared_ptr<CanvasType> Spire::Intersect(const CanvasType& a,
    const CanvasType& b) {
  if(a.GetCompatibility(b) == CanvasType::Compatibility::EQUAL) {
    return a;
  }
  if(dynamic_cast<const NativeType*>(&a) &&
      dynamic_cast<const NativeType*>(&b)) {
    return UnionType::GetEmptyType();
  }
  auto unionTypeA = dynamic_cast<const UnionType*>(&a);
  auto unionTypeB = dynamic_cast<const UnionType*>(&b);
  const NativeType* nativeType;
  if(unionTypeA == nullptr) {
    unionTypeA = unionTypeB;
    nativeType = dynamic_cast<const NativeType*>(&a);
  } else if(unionTypeB == nullptr) {
    nativeType = dynamic_cast<const NativeType*>(&b);
  } else {
    nativeType = nullptr;
  }
  if(nativeType != nullptr) {
    for(const auto& type : unionTypeA->GetCompatibleTypes()) {
      if(nativeType->GetCompatibility(type) ==
          CanvasType::Compatibility::EQUAL) {
        return *nativeType;
      }
    }
    return UnionType::GetEmptyType();
  }
  vector<std::shared_ptr<NativeType>> compatibleTypes;
  for(const auto& typeA : unionTypeA->GetCompatibleTypes()) {
    for(const auto& typeB : unionTypeB->GetCompatibleTypes()) {
      if(typeA.GetCompatibility(typeB) == CanvasType::Compatibility::EQUAL) {
        compatibleTypes.push_back(typeA);
        break;
      }
    }
  }
  return UnionType::Create(make_dereference_view(compatibleTypes));
}

std::shared_ptr<CanvasType> Spire::LeftIntersect(const CanvasType& left,
    const CanvasType& right) {
  if(left.GetCompatibility(right) == CanvasType::Compatibility::EQUAL) {
    return left;
  }
  if(dynamic_cast<const NativeType*>(&left) &&
      dynamic_cast<const NativeType*>(&right)) {
    return left;
  }
  auto unionTypeA = dynamic_cast<const UnionType*>(&left);
  auto unionTypeB = dynamic_cast<const UnionType*>(&right);
  const NativeType* nativeType;
  if(unionTypeA == nullptr) {
    unionTypeA = unionTypeB;
    nativeType = dynamic_cast<const NativeType*>(&left);
  } else if(unionTypeB == nullptr) {
    nativeType = dynamic_cast<const NativeType*>(&right);
  } else {
    nativeType = nullptr;
  }
  if(nativeType != nullptr) {
    for(const auto& type : unionTypeA->GetCompatibleTypes()) {
      if(nativeType->GetCompatibility(type) ==
          CanvasType::Compatibility::EQUAL) {
        return *nativeType;
      }
    }
    return left;
  }
  vector<std::shared_ptr<NativeType>> compatibleTypes;
  for(const auto& typeA : unionTypeA->GetCompatibleTypes()) {
    for(const auto& typeB : unionTypeB->GetCompatibleTypes()) {
      if(typeA.GetCompatibility(typeB) == CanvasType::Compatibility::EQUAL) {
        compatibleTypes.push_back(typeA);
        break;
      }
    }
  }
  if(compatibleTypes.empty()) {
    return left;
  }
  return UnionType::Create(make_dereference_view(compatibleTypes));
}
