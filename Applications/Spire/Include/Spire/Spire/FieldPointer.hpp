#ifndef SPIRE_FIELD_POINTER_HPP
#define SPIRE_FIELD_POINTER_HPP
#include <memory>
#include <typeinfo>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Abstracts over a pointer to a member variable. */
  class FieldPointer {
    public:

      /**
       * Decomposes the type of a pointer to member variable into the type the
       * member variable and the type of the object.
       * @param <T> The type of pointer to member variable to deconstruct.
       */
      template<typename T> struct split_pointer_to_member {};

      template<typename T, typename U>
      struct split_pointer_to_member<U T::*> {
        using field = U;
        using object = T;
      };

      /** Constructs a null pointer. */
      FieldPointer() = default;

      /** Constructs a null pointer. */
      FieldPointer(std::nullptr_t);

      /**
       * Constructs a pointer to an existing member variable.
       * @param pointer The pointer to the member variable.
       */
      template<typename T>
      FieldPointer(T pointer);

      /**
       * Returns <code>true</code> iff this does not represent a null pointer.
       */
      explicit operator bool() const;

      /**
       * Accesses the referenced field within an object.
       * @param <T> The type of object to access.
       * @param <U> The type of member to access.
       * @param object The object whose field is being accessed.
       */
      template<typename U, typename T>
      const U& access(const T& object) const;

      /**
       * Accesses the referenced field within an object.
       * @param <T> The type of object to access.
       * @param <U> The type of member to access.
       * @param object The object whose field is being accessed.
       */
      template<typename U, typename T>
      U& access(T& object) const;

      /** Tests if two pointers point to the same member variable. */
      bool operator ==(const FieldPointer& other) const;

      /** Tests if this is equal to the null pointer. */
      bool operator ==(std::nullptr_t) const;

      bool operator !=(const FieldPointer& other) const = default;

      bool operator !=(std::nullptr_t) const;

    private:
      struct VirtualFieldPointer {
        virtual ~VirtualFieldPointer() = default;
        virtual void* access(void* object, const std::type_info& object_type,
          const std::type_info& member_type) const = 0;
        virtual bool operator ==(const VirtualFieldPointer& other) const = 0;
        bool operator !=(const VirtualFieldPointer& other) const = default;
      };
      template<typename T>
      struct FieldPointerWrapper : VirtualFieldPointer {
        T m_pointer;

        FieldPointerWrapper(T pointer);
        void* access(void* object, const std::type_info& object_type,
          const std::type_info& member_type) const override;
        bool operator ==(const VirtualFieldPointer& other) const override;
      };
      std::shared_ptr<VirtualFieldPointer> m_instance;
  };

  template<typename T>
  FieldPointer::FieldPointer(T pointer)
    : m_instance(std::make_shared<FieldPointerWrapper<T>>(pointer)) {}

  template<typename U, typename T>
  const U& FieldPointer::access(const T& object) const {
    return access<U>(const_cast<T&>(object));
  }

  template<typename U, typename T>
  U& FieldPointer::access(T& object) const {
    if(m_instance) {
      return *static_cast<U*>(
        m_instance->access(&object, typeid(T), typeid(U)));
    }
    throw std::bad_cast();
  }

  template<typename T>
  FieldPointer::FieldPointerWrapper<T>::FieldPointerWrapper(T pointer)
    : m_pointer(pointer) {}

  template<typename T>
  void* FieldPointer::FieldPointerWrapper<T>::access(void* object,
      const std::type_info& object_type,
      const std::type_info& member_type) const {
    using Split = split_pointer_to_member<T>;
    if(object_type != typeid(typename Split::object) ||
        member_type != typeid(typename Split::field)) {
      throw std::bad_cast();
    }
    return &(static_cast<typename Split::object*>(object)->*m_pointer);
  }

  template<typename T>
  bool FieldPointer::FieldPointerWrapper<T>::operator ==(
      const VirtualFieldPointer& other) const {
    if(auto pointer = dynamic_cast<const FieldPointerWrapper*>(&other)) {
      return m_pointer == pointer->m_pointer;
    }
    return false;
  }
}

#endif
