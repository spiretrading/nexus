#include "Spire/Spire/FieldPointer.hpp"

using namespace Spire;

FieldPointer::FieldPointer(std::nullptr_t) {}

FieldPointer::operator bool() const {
  return static_cast<bool>(m_instance);
}

bool FieldPointer::operator ==(const FieldPointer& other) const {
  if(m_instance && other.m_instance) {
    return *m_instance == *other.m_instance;
  } else if(!m_instance) {
    return !other.m_instance;
  }
  return false;
}

bool FieldPointer::operator ==(std::nullptr_t) const {
  return m_instance == nullptr;
}

bool FieldPointer::operator !=(std::nullptr_t) const {
  return !(*this == nullptr);
}
