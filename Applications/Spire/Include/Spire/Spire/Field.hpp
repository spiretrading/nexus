#ifndef SPIRE_FIELD_HPP
#define SPIRE_FIELD_HPP
#include <cstring>
#include <functional>
#include <memory>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Provides a type-erased interface to a member variable. */
  class Field {
    public:

      /**
       * Constructs a Field from a pointer to member variable.
       * @param accessor The pointer to member variable.
       */
      template<typename T, typename M>
      explicit Field(M T::* accessor);

      /**
       * Returns a reference to an object's member variable.
       * @param object The object whose member variable is being accessed.
       * @return A reference to the member variable represented by this Field.
       */
      template<typename M, typename T>
      const M& access(const T& object) const;

      /**
       * Returns a reference to an object's member variable.
       * @param object The object whose member variable is being accessed.
       * @return A reference to the member variable represented by this Field.
       */
      template<typename M, typename T>
      M& access(T& object);

      /** Returns a hash code for this Field. */
      std::size_t get_hash() const;

      /** Tests if this Field represents the same member variable as another. */
      bool operator ==(const Field& field) const;

    private:
      struct VirtualAccessor {
        virtual ~VirtualAccessor() = default;

        virtual void* access(void* object) = 0;
        virtual std::size_t get_hash() const = 0;
        virtual bool operator ==(const VirtualAccessor& accessor) const = 0;
      };
      template<typename T, typename M>
      struct Accessor final : VirtualAccessor {
        using Type = M T::*;
        Type m_accessor;
        std::size_t m_hash;

        Accessor(Type accessor);

        void* access(void* object) override;
        std::size_t get_hash() const override;
        bool operator ==(const VirtualAccessor& accessor) const override;
      };
      std::shared_ptr<VirtualAccessor> m_accessor;
  };

  template<typename T, typename M>
  Field::Field(M T::* accessor)
    : m_accessor(std::make_shared<Accessor<T, M>>(std::move(accessor))) {}

  template<typename M, typename T>
  const M& Field::access(const T& object) const {
    return const_cast<Field*>(this)->access();
  }

  template<typename M, typename T>
  M& Field::access(T& object) {
    return *static_cast<M*>(m_accessor->access(&object));
  }

  template<typename T, typename M>
  Field::Accessor<T, M>::Accessor(Type accessor)
      : m_accessor(std::move(accessor)),
        m_hash(0) {
    std::memcpy(
      &m_hash, &m_accessor, std::min(sizeof(std::size_t), sizeof(Type)));
  }

  template<typename T, typename M>
  void* Field::Accessor<T, M>::access(void* object) {
    return &(static_cast<T*>(object)->*m_accessor);
  }

  template<typename T, typename M>
  std::size_t Field::Accessor<T, M>::get_hash() const {
    return m_hash;
  }

  template<typename T, typename M>
  bool Field::Accessor<T, M>::operator ==(
      const VirtualAccessor& accessor) const {
    if(auto f = dynamic_cast<const Accessor<T, M>*>(&accessor)) {
      return m_accessor == f->m_accessor;
    }
    return false;
  }
}

namespace std {
  template<>
  struct hash<Spire::Field> {
    std::size_t operator ()(const Spire::Field& field) const;
  };
}

#endif
