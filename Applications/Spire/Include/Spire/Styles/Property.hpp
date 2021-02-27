#ifndef SPIRE_STYLES_PROPERTY_HPP
#define SPIRE_STYLES_PROPERTY_HPP
#include <any>
#include <typeindex>
#include <type_traits>
#include <Beam/Utilities/Functional.hpp>
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Stores a generic style property. */
  class Property {
    public:

      /** Stores a BasicProperty. */
      template<typename T, typename G>
      Property(BasicProperty<T, G> property);

      /** Returns the underlying property's type. */
      std::type_index get_type() const;

      /** Casts the underlying property to a specified type. */
      template<typename U>
      const U& as() const;

      /**
       * Applies a callable to the underlying property stored.
       * @param f The callable to apply.
       */
      template<typename F>
      decltype(auto) visit(F&& f) const;

      template<typename F, typename... G>
      decltype(auto) visit(F&& f, G&&... g) const;

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
      std::any m_property;
  };

  template<typename T, typename G>
  Property::Property(BasicProperty<T, G> property)
    : m_property(std::move(property)) {}

  template<typename U>
  const U& Property::as() const {
    return std::any_cast<const U&>(m_property);
  }

  template<typename F>
  decltype(auto) Property::visit(F&& f) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if(m_property.type() == typeid(Parameter)) {
      return std::forward<F>(f)(std::any_cast<const Parameter&>(m_property));
    }
    throw std::bad_any_cast();
  }

  template<typename F, typename... G>
  decltype(auto) Property::visit(F&& f, G&&... g) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if(m_property.type() == typeid(Parameter)) {
      return std::forward<F>(f)(std::any_cast<const Parameter&>(m_property));
    }
    return visit(std::forward<G>(g)...);
  }
}

#endif
