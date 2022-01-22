#include "Spire/Spire/AnyRef.hpp"

using namespace Spire;

AnyRef::AnyRef() noexcept
  : AnyRef(nullptr) {}

AnyRef::AnyRef(std::nullptr_t) noexcept
  : m_ptr(nullptr),
    m_get_type(nullptr),
    m_qualifiers(Qualifiers::NONE) {}

AnyRef::AnyRef(AnyRef&& any) noexcept {
  m_ptr = any.m_ptr;
  m_get_type = any.m_get_type;
  m_qualifiers = any.m_qualifiers;
}

bool AnyRef::has_value() const noexcept {
  return m_ptr != nullptr;
}

const std::type_info& AnyRef::get_type() const noexcept {
  if(!has_value()) {
    return typeid(void);
  }
  return m_get_type();
}

bool AnyRef::is_const() const noexcept {
  if(!has_value()) {
    return false;
  }
  return is_set(m_qualifiers, Qualifiers::CONSTANT);
}

bool AnyRef::is_volatile() const noexcept {
  if(!has_value()) {
    return false;
  }
  return is_set(m_qualifiers, Qualifiers::VOLATILE);
}

bool AnyRef::is_const_volatile() const noexcept {
  if(!has_value()) {
    return false;
  }
  return is_set(m_qualifiers, Qualifiers::CONSTANT) &&
    is_set(m_qualifiers, Qualifiers::VOLATILE);
}

AnyRef& AnyRef::operator =(std::nullptr_t) noexcept {
  m_ptr = nullptr;
  m_get_type = nullptr;
  m_qualifiers = Qualifiers::NONE;
  return *this;
}

AnyRef& AnyRef::operator =(AnyRef&& any) noexcept {
  m_ptr = any.m_ptr;
  m_get_type = any.m_get_type;
  m_qualifiers = any.m_qualifiers;
  return *this;
}

bool AnyRef::is_set(Qualifiers left, Qualifiers right) const {
  return static_cast<Qualifiers>(
    static_cast<std::underlying_type_t<Qualifiers>>(left) &
    static_cast<std::underlying_type_t<Qualifiers>>(right)) !=
    Qualifiers::NONE;
}
