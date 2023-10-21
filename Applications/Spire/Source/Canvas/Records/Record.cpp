#include "Spire/Canvas/Records/Record.hpp"
#include <Beam/Utilities/HashPtime.hpp>
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

std::size_t Spire::hash_value(const Record& record) noexcept {
  auto seed = std::size_t(0);
  for(auto& field : record.GetFields()) {
    hash_combine(seed, std::hash<Record::Field>()(field));
  }
  return seed;
}

std::size_t
    std::hash<Record>::operator ()(const Record& record) const noexcept {
  return hash_value(record);
}
