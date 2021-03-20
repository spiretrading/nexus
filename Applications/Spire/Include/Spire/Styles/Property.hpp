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
      std::any m_property;
      std::function<bool (const Property&, const Property&)> m_is_equal;
  };

  template<typename T, typename G>
  Property::Property(BasicProperty<T, G> property)
    : m_property(std::move(property)),
      m_is_equal([] (const Property& left, const Property& right) {
        if(left.get_type() != right.get_type()) {
          return false;
        }
        return left.as<BasicProperty<T, G>>() ==
          right.as<BasicProperty<T, G>>();
      }) {}

  template<typename U>
  const U& Property::as() const {
    return std::any_cast<const U&>(m_property);
  }

  template<typename F>
  decltype(auto) Property::visit(F&& f) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if constexpr(std::is_invocable_v<std::decay_t<F>>) {
      return std::forward<F>(f)();
    } else if(m_property.type() == typeid(Parameter)) {
      return std::forward<F>(f)(std::any_cast<const Parameter&>(m_property));
    }
    if constexpr(!std::is_invocable_r_v<void, F, const Parameter&>) {
      throw std::bad_any_cast();
    }
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
