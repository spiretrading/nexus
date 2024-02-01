#ifndef SPIRE_STYLES_PROPERTY_HPP
#define SPIRE_STYLES_PROPERTY_HPP
#include <any>
#include <typeindex>
#include <type_traits>
#include <utility>
#include <Beam/Utilities/Functional.hpp>
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Styles/EnumProperty.hpp"
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Stores a generic style property. */
  class Property {
    public:

      /** Stores a BasicProperty. */
      template<typename T, typename G>
      Property(BasicProperty<T, G> property);

      /** Stores an enum property. */
      template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
      Property(T property);

      /** Returns the underlying property's type. */
      std::type_index get_type() const;

      /** Casts the underlying property to a specified type. */
      template<typename U>
      const U& as() const;

      /** Casts the underlying property's expression to a specified type. */
      template<typename U>
      const Expression<U>& expression_as() const;

      /**
       * Applies a callable to the underlying property stored.
       * @param f The callable to apply.
       */
      template<typename F>
      decltype(auto) visit(F&& f) const;

      template<typename F, typename... G>
      decltype(auto) visit(F&& f, G&&... g) const;

      bool operator ==(const Property& property) const;

      bool operator !=(const Property& property) const;

    private:
      template<typename T>
      struct TypeExtractor {};
      template<typename T>
      struct TypeExtractor<Beam::TypeSequence<T>> {
        using type = std::decay_t<T>;
      };
      template<typename T, typename U>
      struct TypeExtractor<Beam::TypeSequence<T, U>> {
        using type = std::decay_t<U>;
      };
      friend struct std::hash<Property>;
      struct BaseEntry {
        virtual ~BaseEntry() = default;
        virtual std::type_index get_type() const = 0;
        virtual std::size_t hash() const = 0;
        virtual const void* expression_as(std::type_index type) const = 0;
        virtual bool operator ==(const BaseEntry& entry) const = 0;
        bool operator !=(const BaseEntry& entry) const;
      };
      template<typename T>
      struct Entry final : BaseEntry {
        using Type = T;
        Type m_property;

        Entry(Type property);
        std::type_index get_type() const override;
        virtual std::size_t hash() const override;
        const void* expression_as(std::type_index type) const override;
        bool operator ==(const BaseEntry& entry) const override;
      };
      std::shared_ptr<BaseEntry> m_entry;
  };

  template<typename T, typename G>
  Property::Property(BasicProperty<T, G> property)
    : m_entry(
        std::make_shared<Entry<BasicProperty<T, G>>>(std::move(property))) {}

  template<typename T, typename>
  Property::Property(T property)
    : Property(EnumProperty<T>(property)) {}

  template<typename U>
  const U& Property::as() const {
    if(m_entry->get_type() != typeid(U)) {
      throw std::bad_any_cast();
    }
    return static_cast<const Entry<U>&>(*m_entry).m_property;
  }

  template<typename U>
  const Expression<U>& Property::expression_as() const {
    return
      *static_cast<const Expression<U>*>(m_entry->expression_as(typeid(U)));
  }

  template<typename F>
  decltype(auto) Property::visit(F&& f) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if constexpr(std::is_invocable_v<std::decay_t<F>>) {
      return std::forward<F>(f)();
    } else if(m_entry->get_type() == typeid(Parameter)) {
      return std::forward<F>(f)(
        static_cast<const Entry<Parameter>&>(*m_entry).m_property);
    }
    if constexpr(!std::is_invocable_r_v<void, F, const Parameter&>) {
      throw std::bad_any_cast();
    }
  }

  template<typename F, typename... G>
  decltype(auto) Property::visit(F&& f, G&&... g) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if(m_entry->get_type() == typeid(Parameter)) {
      return std::forward<F>(f)(
        static_cast<const Entry<Parameter>&>(*m_entry).m_property);
    }
    return visit(std::forward<G>(g)...);
  }

  template<typename T>
  Property::Entry<T>::Entry(Type property)
    : m_property(std::move(property)) {}

  template<typename T>
  std::type_index Property::Entry<T>::get_type() const {
    return typeid(Type);
  }

  template<typename T>
  std::size_t Property::Entry<T>::hash() const {
    return std::hash<Type>()(m_property);
  }

  template<typename T>
  const void* Property::Entry<T>::expression_as(std::type_index type) const {
    if(type != typeid(typename Type::Type)) {
      throw std::bad_any_cast();
    }
    return &m_property.get_expression();
  }

  template<typename T>
  bool Property::Entry<T>::operator ==(const BaseEntry& entry) const {
    return entry.get_type() == typeid(Type) &&
      m_property == static_cast<const Entry<Type>&>(entry).m_property;
  }
}

namespace std {
  template<>
  struct hash<Spire::Styles::Property> {
    std::size_t operator ()(const Spire::Styles::Property& property) const;
  };
}

#endif
