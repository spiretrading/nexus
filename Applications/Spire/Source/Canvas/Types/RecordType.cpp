#include "Spire/Canvas/Types/RecordType.hpp"
#include <Beam/Utilities/Algorithm.hpp>
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

const RecordType& RecordType::GetEmptyRecordType() {
  static auto type = std::shared_ptr<RecordType>(new RecordType());
  return *type;
}

RecordType::Field::Field(std::string name, const NativeType& type)
    : m_name(std::move(name)),
      m_type(type) {}

RecordType::RecordType()
    : m_name("Empty Record") {}

RecordType::RecordType(vector<Field> fields)
    : m_fields(std::move(fields)) {
  if(m_fields.empty()) {
    m_name = "Empty Record";
  } else {
    m_name = "Record";
  }
}

RecordType::RecordType(string name, vector<Field> fields)
    : m_name(name + " Record"),
      m_fields(std::move(fields)) {}

RecordType::operator std::shared_ptr<RecordType> () const {
  std::shared_ptr<NativeType> instance = *this;
  return std::static_pointer_cast<RecordType>(instance);
}

const vector<RecordType::Field>& RecordType::GetFields() const {
  return m_fields;
}

string RecordType::GetName() const {
  return m_name;
}

const type_info& RecordType::GetNativeType() const {
  return typeid(Record);
}

CanvasType::Compatibility RecordType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) != typeid(RecordType)) {
    return Compatibility::NONE;
  }
  auto& recordType = static_cast<const RecordType&>(type);
  auto isEqual = true;
  for(const auto& selfField : m_fields) {
    auto field = FindField(recordType, selfField.m_name);
    if(!field.is_initialized()) {
      return Compatibility::NONE;
    }
    auto compatibility = selfField.m_type->GetCompatibility(*field->m_type);
    if(compatibility != Compatibility::EQUAL) {
      isEqual = false;
    }
    if(!IsCompatible(compatibility)) {
      return Compatibility::NONE;
    }
  }
  if(isEqual && recordType.m_fields.size() == m_fields.size()) {
    return Compatibility::EQUAL;
  }
  return Compatibility::COMPATIBLE;
}

void RecordType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}

boost::optional<const RecordType::Field&> Spire::FindField(
    const RecordType& type, const string& name) {
  return find_if(type.GetFields(), [&] (const RecordType::Field& field) {
    return field.m_name == name;
  });
}

std::shared_ptr<RecordType> Spire::MakeRecordType(
    vector<RecordType::Field> fields) {
  return std::shared_ptr<RecordType>(new RecordType(std::move(fields)));
}

std::shared_ptr<RecordType> Spire::MakeRecordType(string name,
    vector<RecordType::Field> fields) {
  return std::shared_ptr<RecordType>(new RecordType(std::move(name),
    std::move(fields)));
}
