#ifndef SPIRE_ANY_REF_HPP
#define SPIRE_ANY_REF_HPP
#include <any>
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
      requires std::conjunction_v<
        std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
        std::is_copy_constructible<std::decay_t<T>>>
      AnyRef(T& ref) noexcept;

      /** Constructs an AnyRef referencing a const object. */
      template<typename T>
      requires std::conjunction_v<
        std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
        std::is_copy_constructible<std::decay_t<T>>>
      AnyRef(const T& ref) noexcept;

      /** Constructs an AnyRef referencing a volatile object. */
      template<typename T>
      requires std::conjunction_v<
        std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
        std::is_copy_constructible<std::decay_t<T>>>
      AnyRef(volatile T& ref) noexcept;

      /** Constructs an AnyRef referencing a const and volatile object. */
      template<typename T>
      requires std::conjunction_v<
        std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
        std::is_copy_constructible<std::decay_t<T>>>
      AnyRef(const volatile T& ref) noexcept;

      AnyRef(const AnyRef& any) noexcept = default;

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

      AnyRef& operator =(std::nullptr_t) noexcept;

      template<typename T>
      AnyRef& operator =(T& ref) noexcept;

      template<typename T>
      AnyRef& operator =(const T& ref) noexcept;

      template<typename T>
      AnyRef& operator =(volatile T& ref) noexcept;

      template<typename T>
      AnyRef& operator =(const volatile T& ref) noexcept;

      AnyRef& operator =(AnyRef&& any) noexcept;

    private:
      enum class Qualifiers : std::uint8_t {
        NONE = 0,
        CONSTANT = 1,
        VOLATILE = 2
      };
      template<typename T>
      friend const T* any_cast(const AnyRef* any) noexcept;
      template<typename T>
      friend T* any_cast(AnyRef* any) noexcept;
      void* m_ptr;
      std::type_info const& (*m_get_type)();
      Qualifiers m_qualifiers;

      template<typename T>
      const T* cast() const noexcept;
      template<typename T>
      T* cast() noexcept;
      bool is_set(Qualifiers left, Qualifiers right) const;
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
    const auto p = any_cast<T>(&any);
    if (!p) {
      throw std::bad_any_cast();
    }
    return static_cast<const T&>(*p);
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
    auto p = any_cast<T>(&any);
    if (!p) {
      throw std::bad_any_cast();
    }
    return static_cast<T&>(*p);
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
    if(!any) {
      return nullptr;
    }
    return any->cast<T>();
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
    if(!any) {
      return nullptr;
    }
    return any->cast<T>();
  }

  template<typename T>
  requires std::conjunction_v<
    std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
    std::is_copy_constructible<std::decay_t<T>>>
  AnyRef::AnyRef(T& ref) noexcept
      : m_qualifiers(Qualifiers::NONE),
        m_get_type([] () -> std::type_info const& {
          return typeid(std::remove_pointer_t<T>); }) {
    if constexpr(std::is_pointer_v<T>) {
      m_ptr = static_cast<void*>(ref);
    } else {
      m_ptr = static_cast<void*>(&ref);
    }
  }

  template<typename T>
  requires std::conjunction_v<
    std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
    std::is_copy_constructible<std::decay_t<T>>>
  AnyRef::AnyRef(const T& ref) noexcept
      : m_qualifiers(Qualifiers::CONSTANT),
        m_get_type([] () -> std::type_info const& {
          return typeid(std::remove_pointer_t<T>); }) {
    if constexpr(std::is_pointer_v<T>) {
      m_ptr = static_cast<void*>(ref);
    } else {
      m_ptr = const_cast<void*>(static_cast<const void*>(&ref));
    }
  }

  template<typename T>
  requires std::conjunction_v<
    std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
    std::is_copy_constructible<std::decay_t<T>>>
  AnyRef::AnyRef(volatile T& ref) noexcept
      : m_qualifiers(Qualifiers::VOLATILE),
        m_get_type([] () -> std::type_info const& {
          return typeid(std::remove_pointer_t<T>); }) {
    if constexpr(std::is_pointer_v<T>) {
      m_ptr = static_cast<void*>(ref);
    } else {
      m_ptr = const_cast<void*>(static_cast<volatile void*>(&ref));
    }
  }

  template<typename T>
  requires std::conjunction_v<
    std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
    std::is_copy_constructible<std::decay_t<T>>>
  AnyRef::AnyRef(const volatile T& ref) noexcept
      : m_qualifiers(static_cast<Qualifiers>(
          static_cast<std::underlying_type_t<Qualifiers>>(
            Qualifiers::CONSTANT) |
          static_cast<std::underlying_type_t<Qualifiers>>(
            Qualifiers::VOLATILE))),
        m_get_type([] () -> std::type_info const& {
          return typeid(std::remove_pointer_t<T>); }) {
    if constexpr(std::is_pointer_v<T>) {
      m_ptr = static_cast<void*>(ref);
    } else {
      m_ptr = const_cast<void*>(static_cast<const volatile void*>(&ref));
    }
  }

  template<typename T>
  AnyRef& AnyRef::operator =(T& ref) noexcept {
    *this = AnyRef(ref);
    return *this;
  }

  template<typename T>
  AnyRef& AnyRef::operator =(const T& ref) noexcept {
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

  template<typename T>
  const T* AnyRef::cast() const noexcept {
    if(m_get_type() != typeid(T)) {
      return nullptr;
    }
    if(std::is_const_v<T> == is_const() &&
        std::is_volatile_v<T> == is_volatile()) {
      return static_cast<const T*>(m_ptr);
    } else if constexpr(std::is_const_v<T>) {
      if(std::is_volatile_v<T> == is_volatile()) {
        return static_cast<const T*>(m_ptr);
      }
    }
    return nullptr;
  }

  template<typename T>
  T* AnyRef::cast() noexcept {
    return const_cast<T*>(static_cast<const AnyRef*>(this)->cast<T>());
  }
}

#endif
