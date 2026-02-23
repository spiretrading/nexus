#include "Spire/Canvas/Records/Record.hpp"
#include <Beam/Utilities/HashPosixTimeTypes.hpp>
#include <boost/functional/hash.hpp>

using namespace boost;
using namespace Spire;

Record::Record(std::vector<Field> fields)
  : m_fields(std::move(fields)) {}

const std::vector<Record::Field>& Record::GetFields() const {
  return m_fields;
}

Record::AddField::AddField(std::vector<Field>& fields)
  : m_fields(fields) {}

std::size_t
    std::hash<Record>::operator ()(const Record& record) const noexcept {
  auto seed = std::size_t(0);
  for(auto& field : record.GetFields()) {
    hash_combine(seed, apply_visitor([] (const auto& field) {
      return std::hash<std::remove_cvref_t<decltype(field)>>()(field);
    }, field));
  }
  return seed;
}
