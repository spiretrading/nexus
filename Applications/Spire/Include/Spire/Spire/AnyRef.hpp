#ifndef SPIRE_ANY_REF_HPP
#define SPIRE_ANY_REF_HPP
#include <any>
#include <typeindex>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a generic type-safe reference to an existing object that
   * preserves cv-qualifiers.
   */
  class AnyRef {
    public:

      /** Constructs an empty object. */
      AnyRef() noexcept;

      /** Constructs an empty object. */
      AnyRef(std::nullptr_t) noexcept;

      /** Constructs an AnyRef referencing a non-const, non-volatile object. */
      template<typename T>
      AnyRef(T& ref) noexcept;

      /** Constructs an AnyRef referencing a const object. */
      template<typename T>
      AnyRef(const T& ref) noexcept;

      /** Constructs an AnyRef referencing a volatile object. */
      template<typename T>
      AnyRef(volatile T& ref) noexcept;

      /** Constructs an AnyRef referencing a const and volatile object. */
      template<typename T>
      AnyRef(const volatile T& ref) noexcept;

      AnyRef(const AnyRef& any) noexcept = default;

      AnyRef(AnyRef& any) noexcept = default;

      AnyRef(AnyRef&& any) noexcept;

      /** Returns <code>true</code> iff this object is not empty. */
      bool has_value() const noexcept;

      /** Returns the typeid of the referenced object. */
      const std::type_info& get_type() const noexcept;

      /** Returns <code>true</code> iff the referenced object is const. */
      bool is_const() const noexcept;

      /** Returns <code>true</code> iff the referenced object is volatile. */
      bool is_volatile() const noexcept;

      /**
       * Returns <code>true</code> iff the referenced object is both const and
       * volatile.
       */
      bool is_const_volatile() const noexcept;

      AnyRef& operator =(const AnyRef& any) noexcept = default;

      AnyRef& operator =(AnyRef& any) noexcept = default;

      AnyRef& operator =(std::nullptr_t) noexcept;

      template<typename T>
      AnyRef& operator =(const T& ref) noexcept;

      template<typename T>
      AnyRef& operator =(T& ref) noexcept;

      template<typename T>
      AnyRef& operator =(volatile T& ref) noexcept;

      template<typename T>
      AnyRef& operator =(const volatile T& ref) noexcept;

      AnyRef& operator =(AnyRef&& any) noexcept;

    private:
      enum class Qualifiers : std::uint8_t {
        NONE = 0,
        CONSTANT = 1,
        VOLATILE = 2,
        CONST_VOLATILE = 3
      };
      void* m_ptr;
      const std::type_info* m_type;
      Qualifiers m_qualifiers;

      template<typename T>
      friend T* any_cast(AnyRef* any) noexcept;
      static bool is_set(Qualifiers a, Qualifiers b);
      AnyRef(void* ptr, const std::type_info& type, Qualifiers qualifiers);
  };

  /**
   * Returns a const reference to the value contained by an AnyRef.
   * @param <T> The type of the reference to access.
   * @param any The AnyRef whose value is being accessed.
   * @throws <code>std::bad_any_cast</code> iff the value contained by
   *         <i>any</i> is not compatible with <i>T</i>.
   */
  template<typename T>
  const T& any_cast(const AnyRef& any) {
    if(auto p = any_cast<T>(&any)) {
      return *p;
    }
    throw std::bad_any_cast();
  }

  /**
   * Returns a reference to the value contained by an AnyRef.
   * @param <T> The type of the reference to access.
   * @param any The AnyRef whose value is being accessed.
   * @throws <code>std::bad_any_cast</code> iff the value contained by
   *         <i>any</i> is not compatible with <i>T</i> or the AnyRef is
   *         referencing a const value.
   */
  template<typename T>
  T& any_cast(AnyRef& any) {
    if(auto p = any_cast<T>(&any)) {
      return *p;
    }
    throw std::bad_any_cast();
  }

  /**
   * Returns a pointer to the value contained by an AnyRef.
   * @param <T> The type of the pointer to access.
   * @param any The AnyRef whose value is being accessed.
   * @return <code>nullptr</code> iff the value contained by <i>any</i> is not
   *         compatible with <i>T</i>.
   */
  template<typename T>
  const T* any_cast(const AnyRef* any) noexcept {
    return any_cast<T>(const_cast<AnyRef*>(any));
  }

  /**
   * Returns a pointer to the value contained by an AnyRef.
   * @param <T> The type of the pointer to access.
   * @param any The AnyRef whose value is being accessed.
   * @return <code>nullptr</code> iff the value contained by
   *         <i>any</i> is not compatible with <i>T</i> or the AnyRef is
   *         referencing a const value.
   */
  template<typename T>
  T* any_cast(AnyRef* any) noexcept {
    if(any->get_type() != typeid(T) ||
        any->is_const() && !std::is_const_v<T> ||
        any->is_volatile() != std::is_volatile_v<T>) {
      return nullptr;
    }
    return static_cast<T*>(any->m_ptr);
  }

  template<typename T>
  AnyRef::AnyRef(T& ref) noexcept
    : AnyRef(&ref, typeid(T), Qualifiers::NONE) {}

  template<typename T>
  AnyRef::AnyRef(const T& ref) noexcept
    : AnyRef(const_cast<T*>(&ref), typeid(T), Qualifiers::CONSTANT) {}

  template<typename T>
  AnyRef::AnyRef(volatile T& ref) noexcept
    : AnyRef(const_cast<T*>(&ref), typeid(T), Qualifiers::VOLATILE) {}

  template<typename T>
  AnyRef::AnyRef(const volatile T& ref) noexcept
    : AnyRef(const_cast<T*>(&ref), typeid(T), Qualifiers::CONST_VOLATILE) {}

  template<typename T>
  AnyRef& AnyRef::operator =(const T& ref) noexcept {
    *this = AnyRef(ref);
    return *this;
  }

  template<typename T>
  AnyRef& AnyRef::operator =(T& ref) noexcept {
    *this = AnyRef(ref);
    return *this;
  }

  template<typename T>
  AnyRef& AnyRef::operator =(volatile T& ref) noexcept {
    *this = AnyRef(ref);
    return *this;
  }

  template<typename T>
  AnyRef& AnyRef::operator =(const volatile T& ref) noexcept {
    *this = AnyRef(ref);
    return *this;
  }
}

#endif
