#include "Spire/Spire/AnyRef.hpp"

using namespace Spire;

AnyRef::AnyRef() noexcept {}

AnyRef::AnyRef(std::nullptr_t) noexcept {}

AnyRef::AnyRef(AnyRef&& any) noexcept {
  if (any.has_value()) {
    m_ref = std::move(any.m_ref);
	}
}

bool AnyRef::has_value() const noexcept {
  return m_ref != nullptr;
}

const std::type_info& AnyRef::get_type() const noexcept {
  if(!has_value()) {
    return typeid(void);
  }
  return m_ref->get_type();
}

bool AnyRef::is_const() const noexcept {
  if(!has_value()) {
    return false;
  }
  return m_ref->is_const();
}

bool AnyRef::is_volatile() const noexcept {
  if(!has_value()) {
    return false;
  }
  return m_ref->is_volatile();
}

bool AnyRef::is_const_volatile() const noexcept {
  if(!has_value()) {
    return false;
  }
  return m_ref->is_const_volatile();
}

AnyRef& AnyRef::operator =(std::nullptr_t) noexcept {
  m_ref.reset();
  return *this;
}

AnyRef& AnyRef::operator =(AnyRef&& any) noexcept {
  m_ref = std::move(any.m_ref);
  return *this;
}
