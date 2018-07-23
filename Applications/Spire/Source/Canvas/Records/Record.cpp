#include "Spire/Canvas/Records/Record.hpp"

using namespace Spire;
using namespace std;

Record::Record(vector<Field> fields)
    : m_fields(std::move(fields)) {}

const vector<Record::Field>& Record::GetFields() const {
  return m_fields;
}

bool Record::operator ==(const Record& rhs) const {
  return m_fields == rhs.m_fields;
}

bool Record::operator !=(const Record& rhs) const {
  return !(*this == rhs);
}
