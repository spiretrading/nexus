#include "Spire/Spire/AnyRef.hpp"

using namespace Spire;

const AnyRef::AnyTypeInfo& AnyRef::AnyTypeInfo::get() {
  static auto instance = AnyTypeInfo();
  return instance;
}

const std::type_info& AnyRef::AnyTypeInfo::get_type(void* ptr) const noexcept {
  return static_cast<std::any*>(ptr)->type();
}

std::any AnyRef::AnyTypeInfo::to_any(void* ptr) const noexcept {
  return *static_cast<std::any*>(ptr);
}

AnyRef::AnyRef() noexcept
  : AnyRef(nullptr) {}

AnyRef::AnyRef(std::nullptr_t) noexcept
  : AnyRef(nullptr, TypeInfo<void>::get(), Qualifiers::NONE) {}

AnyRef::AnyRef(std::any& value) noexcept
  : AnyRef(&const_cast<std::any&>(value), AnyTypeInfo::get(),
      Qualifiers::NONE) {}

AnyRef::AnyRef(const std::any& value) noexcept
  : AnyRef(&const_cast<std::any&>(value), AnyTypeInfo::get(),
      Qualifiers::CONSTANT) {}

AnyRef::AnyRef(AnyRef&& any) noexcept
  : AnyRef(std::exchange(any.m_ptr, nullptr),
      *std::exchange(any.m_type, &TypeInfo<void>::get()),
      std::exchange(any.m_qualifiers, Qualifiers::NONE)) {}

bool AnyRef::has_value() const noexcept {
  return m_ptr != nullptr;
}

const std::type_info& AnyRef::get_type() const noexcept {
  return m_type->get_type(m_ptr);
}

bool AnyRef::is_const() const noexcept {
  return is_set(m_qualifiers, Qualifiers::CONSTANT);
}

bool AnyRef::is_volatile() const noexcept {
  return is_set(m_qualifiers, Qualifiers::VOLATILE);
}

bool AnyRef::is_const_volatile() const noexcept {
  return is_const() && is_volatile();
}

AnyRef& AnyRef::operator =(std::nullptr_t) noexcept {
  *this = AnyRef();
  return *this;
}

AnyRef& AnyRef::operator =(AnyRef&& any) noexcept {
  m_ptr = std::exchange(any.m_ptr, nullptr);
  m_type = std::exchange(any.m_type, &TypeInfo<void>::get());
  m_qualifiers = std::exchange(any.m_qualifiers, Qualifiers::NONE);
  return *this;
}

bool AnyRef::is_set(Qualifiers a, Qualifiers b) {
  return static_cast<Qualifiers>(std::underlying_type_t<Qualifiers>(a) &
    std::underlying_type_t<Qualifiers>(b)) != Qualifiers::NONE;
}

AnyRef::AnyRef(void* ptr, const BaseTypeInfo& type, Qualifiers qualifiers)
  : m_ptr(ptr),
    m_type(&type),
    m_qualifiers(qualifiers) {}

std::any Spire::to_any(const AnyRef& any) noexcept {
  return any.m_type->to_any(any.m_ptr);
}