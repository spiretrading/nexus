#include "Spire/Canvas/Types/UnionType.hpp"
#include <algorithm>
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/OrderStatusType.hpp"
#include "Spire/Canvas/Types/OrderTypeType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/SequenceType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/Canvas/Types/TimeInForceType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"
#include "Spire/Canvas/Types/VenueType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  std::shared_ptr<UnionType> MakeAnyType() {
    vector<std::shared_ptr<NativeType>> types;
    types.push_back(BooleanType::GetInstance());
    types.push_back(CurrencyType::GetInstance());
    types.push_back(DateTimeType::GetInstance());
    types.push_back(DecimalType::GetInstance());
    types.push_back(DestinationType::GetInstance());
    types.push_back(DurationType::GetInstance());
    types.push_back(IntegerType::GetInstance());
    types.push_back(MoneyType::GetInstance());
    types.push_back(OrderReferenceType::GetInstance());
    types.push_back(OrderStatusType::GetInstance());
    types.push_back(OrderTypeType::GetInstance());
    types.push_back(RecordType::GetEmptyRecordType());
    types.push_back(SecurityType::GetInstance());
    types.push_back(SequenceType::GetInstance());
    types.push_back(SideType::GetInstance());
    types.push_back(TextType::GetInstance());
    types.push_back(TimeInForceType::GetInstance());
    types.push_back(TimeRangeType::GetInstance());
    types.push_back(VenueType::GetInstance());
    return std::static_pointer_cast<UnionType>(UnionType::Create(
      make_dereference_view(types), "Any"));
  }

  std::shared_ptr<UnionType> MakeAnyValueType() {
    vector<std::shared_ptr<NativeType>> types;
    types.push_back(BooleanType::GetInstance());
    types.push_back(CurrencyType::GetInstance());
    types.push_back(DateTimeType::GetInstance());
    types.push_back(DecimalType::GetInstance());
    types.push_back(DestinationType::GetInstance());
    types.push_back(DurationType::GetInstance());
    types.push_back(IntegerType::GetInstance());
    types.push_back(MoneyType::GetInstance());
    types.push_back(OrderStatusType::GetInstance());
    types.push_back(OrderTypeType::GetInstance());
    types.push_back(RecordType::GetEmptyRecordType());
    types.push_back(SecurityType::GetInstance());
    types.push_back(SequenceType::GetInstance());
    types.push_back(SideType::GetInstance());
    types.push_back(TextType::GetInstance());
    types.push_back(TimeInForceType::GetInstance());
    types.push_back(TimeRangeType::GetInstance());
    types.push_back(VenueType::GetInstance());
    return std::static_pointer_cast<UnionType>(UnionType::Create(
      make_dereference_view(types), "Any Value"));
  }
}

const UnionType& UnionType::GetEmptyType() {
  static auto type = std::static_pointer_cast<UnionType>(
    UnionType::Create(std::vector<NativeType>()));
  return *type;
}

const UnionType& UnionType::GetAnyType() {
  static auto type = MakeAnyType();
  return *type;
}

const UnionType& UnionType::GetAnyValueType() {
  static auto type = MakeAnyValueType();
  return *type;
}

std::shared_ptr<CanvasType> UnionType::Create(
    const View<NativeType>& compatibleTypes) {
  vector<std::shared_ptr<NativeType>> filteredTypes;
  for(const auto& compatibleType : compatibleTypes) {
    bool foundEqualType = false;
    for(const auto& filteredType : filteredTypes) {
      if(filteredType->GetCompatibility(compatibleType) ==
          Compatibility::EQUAL) {
        foundEqualType = true;
        break;
      }
    }
    if(!foundEqualType) {
      filteredTypes.push_back(compatibleType);
    }
  }
  if(filteredTypes.size() == 1) {
    return filteredTypes.front();
  }
  sort(filteredTypes.begin(), filteredTypes.end(),
    [] (const auto& left, const auto& right) {
      return left->GetName() < right->GetName();
    });
  string name;
  if(filteredTypes.empty()) {
    name = "None";
  } else {
    name = "Union of ";
    for(size_t i = 0; i < filteredTypes.size() - 1; ++i) {
      name += filteredTypes[i]->GetName() + ", ";
    }
    name += filteredTypes.back()->GetName();
  }
  return Create(make_dereference_view(filteredTypes), std::move(name));
}

std::shared_ptr<CanvasType> UnionType::Create(
    const View<NativeType>& compatibleTypes, string name) {
  vector<std::shared_ptr<NativeType>> filteredTypes;
  for(const auto& compatibleType : compatibleTypes) {
    bool foundEqualType = false;
    for(const auto& filteredType : filteredTypes) {
      if(filteredType->GetCompatibility(compatibleType) ==
          Compatibility::EQUAL) {
        foundEqualType = true;
        break;
      }
    }
    if(!foundEqualType) {
      filteredTypes.push_back(compatibleType);
    }
  }
  if(filteredTypes.size() == 1) {
    return filteredTypes.front();
  }
  return std::shared_ptr<UnionType>(new UnionType(filteredTypes, name));
}

View<NativeType> UnionType::GetCompatibleTypes() const {
  return make_dereference_view(m_compatibleTypes);
}

string UnionType::GetName() const {
  return m_name;
}

CanvasType::Compatibility UnionType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(UnionType)) {
    auto& unionType = static_cast<const UnionType&>(type);
    for(const auto& compatibleType : unionType.m_compatibleTypes) {
      if(!IsCompatible(*this, *compatibleType)) {
        return Compatibility::NONE;
      }
    }
    if(unionType.m_compatibleTypes.size() == m_compatibleTypes.size()) {
      return Compatibility::EQUAL;
    }
    return Compatibility::COMPATIBLE;
  }
  auto compatibility = Compatibility::NONE;
  for(const auto& compatibleType : m_compatibleTypes) {
    compatibility = min<Compatibility>(Compatibility::COMPATIBLE,
      max<Compatibility>(compatibility,
      compatibleType->GetCompatibility(type)));
    if(compatibility == Compatibility::COMPATIBLE) {
      break;
    }
  }
  return compatibility;
}

void UnionType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}

UnionType::UnionType(vector<std::shared_ptr<NativeType>> compatibleTypes,
    string name)
    : m_compatibleTypes(std::move(compatibleTypes)),
      m_name(std::move(name)) {}
