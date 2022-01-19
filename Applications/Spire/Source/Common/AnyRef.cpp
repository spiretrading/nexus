#include "Spire/Spire/AnyRef.hpp"

using namespace Spire;

AnyRef::AnyRef() noexcept {}

AnyRef::AnyRef(std::nullptr_t) noexcept {}

AnyRef::AnyRef(AnyRef&& any) noexcept {}

const void* AnyRef::get() const noexcept {
  return nullptr;
}

void* AnyRef::get() noexcept {
  return nullptr;
}

bool AnyRef::has_value() const noexcept {
  return false;
}

const std::type_info& AnyRef::get_type() const noexcept {
  return typeid(void);
}

bool AnyRef::is_const() const noexcept {
  return false;
}

bool AnyRef::is_volatile() const noexcept {
  return false;
}

bool AnyRef::is_const_volatile() const noexcept {
  return false;
}

AnyRef& AnyRef::operator =(std::nullptr_t) noexcept {
  return *this;
}

AnyRef& AnyRef::operator =(AnyRef&& any) noexcept {
  return *this;
}
