#ifndef SPIRE_STYLES_COMPOSITE_PROPERTY_HPP
#define SPIRE_STYLES_COMPOSITE_PROPERTY_HPP
#include <tuple>
#include <type_traits>
#include <utility>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** A property that composes multiple other properties. */
  template<typename... T>
  class CompositeProperty {
    public:

      /**
       * Constructs a CompositeProperty by initializing of all its components.
       * @param properties The component properties.
       */
      CompositeProperty(T... properties);

      /** Returns one of the component properties. */
      template<typename U>
      const U& get() const;

    private:
      std::tuple<T...> m_properties;
  };

  /** Type trait checking whether a type represents a CompositeProperty. */
  template<typename T>
  struct is_composite_property : std::false_type {};

  template<typename... T>
  struct is_composite_property<CompositeProperty<T...>> : std::true_type {};

  /** Type trait checking whether a type represents a CompositeProperty. */
  template<typename T>
  constexpr auto is_composite_property_v = is_composite_property<T>::value;

  /**
   * Applies a callable to every component of a CompositeProperty.
   * @param property The CompositeProperty to apply the callable to.
   * @param f The callable to apply to each component.
   */
  template<typename... T, typename F>
  void for_each(const CompositeProperty<T...>& property, const F& f) {
    (f(property.get<T>()), ...);
  }

  /**
   * Applies a callable to every component of a CompositeProperty.
   * @param property The CompositeProperty to apply the callable to.
   * @param f The callable to apply to each component.
   */
  template<typename... T, typename F>
  void for_each(CompositeProperty<T...>& property, const F& f) {
    (f(property.get<T>()), ...);
  }

  /**
   * Applies a callable to every component of a CompositeProperty.
   * @param property The CompositeProperty to apply the callable to.
   * @param f The callable to apply to each component.
   */
  template<typename... T, typename F>
  void for_each(CompositeProperty<T...>&& property, const F& f) {
    (f(std::move(property.get<T>())), ...);
  }

  template<typename... T>
  CompositeProperty<T...>::CompositeProperty(T... properties)
    : m_properties(std::move(properties)...) {}

  template<typename... T>
  template<typename U>
  const U& CompositeProperty<T...>::get() const {
    return std::get<U>(m_properties);
  }
}

#endif
