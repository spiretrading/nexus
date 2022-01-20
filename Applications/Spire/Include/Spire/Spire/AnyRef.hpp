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
      struct AnyRefBase {
        virtual void* get() = 0;
        virtual const void* get_const() const = 0;
        virtual volatile void* get_volatile() const = 0;
        virtual const volatile void* get_const_volatile() const = 0;
        virtual const std::type_info& get_type() const = 0;
        virtual constexpr bool is_const() const = 0;
        virtual constexpr bool is_volatile() const = 0;
        virtual constexpr bool is_const_volatile() const = 0;
        virtual ~AnyRefBase() = default;
      };
      template<typename T>
      struct AnyRefImpl : AnyRefBase {
        T& m_t;

        explicit AnyRefImpl(T& t);
        void* get() override;
        const void* get_const() const override;
        volatile void* get_volatile() const override;
        const volatile void* get_const_volatile() const override;
        const std::type_info& get_type() const override;
        constexpr bool is_const() const override;
        constexpr bool is_volatile() const override;
        constexpr bool is_const_volatile() const override;
      };
      template<typename T>
      friend const T* any_cast(const AnyRef* any) noexcept;
      template<typename T>
      friend T* any_cast(AnyRef* any) noexcept;
      std::shared_ptr<AnyRefBase> m_ref;

      template<typename T>
      const T* cast() const noexcept;
      template<typename T>
      T* cast() noexcept;
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
    : m_ref(std::make_shared<AnyRefImpl<T>>(ref)) {}

  template<typename T>
  requires std::conjunction_v<
    std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
    std::is_copy_constructible<std::decay_t<T>>>
  AnyRef::AnyRef(const T& ref) noexcept
    : m_ref(std::make_shared<AnyRefImpl<const T>>(ref)) {}

  template<typename T>
  requires std::conjunction_v<
    std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
    std::is_copy_constructible<std::decay_t<T>>>
  AnyRef::AnyRef(volatile T& ref) noexcept
    : m_ref(std::make_shared<AnyRefImpl<volatile T>>(ref)) {}

  template<typename T>
  requires std::conjunction_v<
    std::negation<std::is_same<std::decay_t<T>, AnyRef>>,
    std::is_copy_constructible<std::decay_t<T>>>
  AnyRef::AnyRef(const volatile T& ref) noexcept
    : m_ref(std::make_shared<AnyRefImpl<const volatile T>>(ref)) {}

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
    if(get_type() != typeid(T)) {
      return nullptr;
    }
    if constexpr(std::is_const_v<T>) {
      if constexpr(std::is_volatile_v<T>) {
        if(is_const() && is_volatile()) {
          return static_cast<T*>(m_ref->get_const_volatile());
        } else if(is_volatile()) {
          return static_cast<T*>(m_ref->get_volatile());
        }
      } else {
        if(is_const()) {
          return static_cast<T*>(m_ref->get_const());
        } else if(!is_volatile()) {
          return static_cast<T*>(m_ref->get());
        }
      }
    } else {
      if constexpr(std::is_volatile_v<T>) {
        if(is_volatile()) {
          return static_cast<T*>(m_ref->get_volatile());
        }
      } else {
        if(!is_volatile()) {
          return static_cast<T*>(m_ref->get());
        }
      }
    }
    return nullptr;
  }

  template<typename T>
  T* AnyRef::cast() noexcept {
    return const_cast<T*>(static_cast<const AnyRef*>(this)->cast<T>());
  }

  template<typename T>
  AnyRef::AnyRefImpl<T>::AnyRefImpl(T& t) : m_t(t) {}

  template<typename T>
  void* AnyRef::AnyRefImpl<T>::get() {
    if constexpr(std::is_const_v<T> || std::is_volatile_v<T>) {
      return nullptr;
    } else if constexpr(std::is_pointer_v<T>) {
      return reinterpret_cast<void*>(m_t);
    } else {
      return reinterpret_cast<void*>(&m_t);
    }
  }

  template<typename T>
  const void* AnyRef::AnyRefImpl<T>::get_const() const {
    if constexpr(std::is_const_v<T> && !std::is_volatile_v<T>) {
      if constexpr(std::is_pointer_v<T>) {
        return reinterpret_cast<const void*>(m_t);
      } else {
        return reinterpret_cast<const void*>(&m_t);
      }
    } else {
      return nullptr;
    }
  }

  template<typename T>
  volatile void* AnyRef::AnyRefImpl<T>::get_volatile() const {
    if constexpr(!std::is_const_v<T> && std::is_volatile_v<T>) {
      if constexpr(std::is_pointer_v<T>) {
        return reinterpret_cast<volatile void*>(m_t);
      } else {
        return reinterpret_cast<volatile void*>(&m_t);
      }
    } else {
      return nullptr;
    }
  }

  template<typename T>
  const volatile void* AnyRef::AnyRefImpl<T>::get_const_volatile() const {
    if constexpr(std::is_const_v<T> && std::is_volatile_v<T>) {
      if constexpr(std::is_pointer_v<T>) {
        return reinterpret_cast<const volatile void*>(m_t);
      } else {
        return reinterpret_cast<const volatile void*>(&m_t);
      }
    } else {
      return nullptr;
    }
  }

  template<typename T>
  const std::type_info& AnyRef::AnyRefImpl<T>::get_type() const {
    return typeid(std::remove_reference_t<std::remove_pointer_t<T>>);
  }

  template<typename T>
  constexpr bool AnyRef::AnyRefImpl<T>::is_const() const {
    return std::is_const_v<T>;
  }

  template<typename T>
  constexpr bool AnyRef::AnyRefImpl<T>::is_volatile() const {
    return std::is_volatile_v<T>;
  }

  template<typename T>
  constexpr bool AnyRef::AnyRefImpl<T>::is_const_volatile() const {
    return is_const() && is_volatile();
  }
}

#endif
