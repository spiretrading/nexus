#ifndef SPIRE_STYLES_SELECTOR_HPP
#define SPIRE_STYLES_SELECTOR_HPP
#include <any>
#include <typeindex>
#include <type_traits>
#include <Beam/Utilities/Functional.hpp>
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects the widget to apply a style rule to. */
  class Selector {
    public:

      /** Constructs a Selector for any component. */
      Selector(Any any);

      /**
       * Constructs a Selector for a StateSelector.
       * @param state The state to represent.
       */
      template<typename T, typename G>
      Selector(StateSelector<T, G> state);

      /**
       * Constructs a Selector for a StateSelector.
       * @param state The state to represent.
       */
      template<typename G>
      Selector(StateSelector<void, G> state);

      /**
       * Constructs a Selector for a NotSelector.
       * @param selector The selector to represent.
       */
      Selector(NotSelector selector);

      /**
       * Constructs a Selector for an AndSelector.
       * @param selector The selector to represent.
       */
      Selector(AndSelector selector);

      /**
       * Constructs a Selector for an OrSelector.
       * @param selector The selector to represent.
       */
      Selector(OrSelector selector);

      /**
       * Constructs a Selector for an IsASelector.
       * @param selector The selector to represent.
       */
      Selector(IsASelector selector);

      /**
       * Constructs a Selector for a DescendantSelector.
       * @param selector The selector to represent.
       */
      Selector(DescendantSelector selector);

      /** Returns the underlying selector's type. */
      std::type_index get_type() const;

      /** Casts the underlying selector to a specified type. */
      template<typename U>
      const U& as() const;

      /** Tests if another Selector matches this one. */
      bool is_match(const Selector& selector) const;

      /**
       * Applies a callable to the underlying selector.
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
      std::any m_selector;
      std::function<bool (const Selector&)> m_matcher;
  };

  template<typename T, typename G>
  Selector::Selector(StateSelector<T, G> state)
    : m_selector(std::move(state)),
      m_matcher([this] (const Selector& selector) {
        if(selector.get_type() != typeid(StateSelector<T, G>)) {
          return false;
        }
        auto& left = as<StateSelector<T, G>();
        auto& right = selector.as<StateSelector<T, G>>();
        return left.get_data() == right.get_data();
      }) {}

  template<typename G>
  Selector::Selector(StateSelector<void, G> state)
    : m_selector(std::move(state)),
      m_matcher([this] (const Selector& selector) {
        return selector.get_type() == typeid(StateSelector<void, G>);
      }) {}

  template<typename U>
  const U& Selector::as() const {
    return std::any_cast<const U&>(m_selector);
  }

  template<typename F>
  decltype(auto) Selector::visit(F&& f) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if(m_selector.type() == typeid(Parameter)) {
      return std::forward<F>(f)(std::any_cast<const Parameter&>(m_selector));
    }
    throw std::bad_any_cast();
  }

  template<typename F, typename... G>
  decltype(auto) Selector::visit(F&& f, G&&... g) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if(m_selector.type() == typeid(Parameter)) {
      return std::forward<F>(f)(std::any_cast<const Parameter&>(m_selector));
    }
    return visit(std::forward<G>(g)...);
  }
}

#endif
