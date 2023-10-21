#include "Spire/Canvas/Records/Record.hpp"
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

template<>
struct std::hash<Record> {
  std::size_t operator()(const Record& record) const noexcept {
    auto seed = std::size_t(0);
    for(auto& field : record.GetFields()) {
      hash_combine(seed, boost::hash<Record::Field>()(field));
    }
    return seed;
  }
};
