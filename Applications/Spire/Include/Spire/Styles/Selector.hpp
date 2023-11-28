#ifndef SPIRE_STYLES_SELECTOR_HPP
#define SPIRE_STYLES_SELECTOR_HPP
#include <any>
#include <functional>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <Beam/Utilities/Functional.hpp>
#include "Spire/Styles/SelectConnection.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Type trait indicating whether a type satisfies the Selector model. */
  template<typename T, typename = void>
  struct is_selector_t : std::false_type {};

  template<typename T>
  struct is_selector_t<T, std::enable_if_t<std::is_same_v<
    decltype(select(std::declval<const T&>(), std::declval<const Stylist&>(),
      std::declval<const SelectionUpdateSignal&>())), SelectConnection>>> :
    std::true_type {};

  template<typename T>
  constexpr auto is_selector_v = is_selector_t<T>::value;

  /** Selects the widget to apply a style rule to. */
  class Selector {
    public:

      template<typename T, typename = std::enable_if_t<is_selector_v<T>>>
      Selector(T selector);

      Selector(const Selector&) = default;

      Selector(Selector&&) = default;

      /** Returns the underlying selector's type. */
      std::type_index get_type() const;

      /** Casts the underlying selector to a specified type. */
      template<typename U>
      const U& as() const;

      /**
       * Applies a callable to the underlying selector.
       * @param f The callable to apply.
       */
      template<typename F>
      decltype(auto) visit(F&& f) const;

      template<typename F, typename... G>
      decltype(auto) visit(F&& f, G&&... g) const;

      bool operator ==(const Selector& selector) const;

      bool operator !=(const Selector& selector) const;

      Selector& operator =(const Selector&) = default;

      Selector& operator =(Selector&&) = default;

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
      friend struct std::hash<Selector>;
      friend SelectConnection select(
        const Selector&, const Stylist&, const SelectionUpdateSignal&);
      struct Operations {
        std::function<bool (const Selector&, const Selector&)> m_is_equal;
        std::function<SelectConnection (
          const Selector&, const Stylist&, const SelectionUpdateSignal&)>
          m_select;
        std::function<std::size_t (const Selector&)> m_hash;
      };
      static std::unordered_map<std::type_index, Operations> m_operations;
      std::any m_selector;
  };

  /**
   * Returns the set of Stylists selected by a Selector. Updates to the
   * selection ared provided through a specified callback.
   * @param selector The Selector used to select Stylists.
   * @param base The Stylist used as the base of the selection.
   * @param on_update The callable to invoke when the selection updates.
   * @return A scoped connection used to receive selection updates.
   */
  SelectConnection select(const Selector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  template<typename T, typename>
  Selector::Selector(T selector)
      : m_selector(std::move(selector)) {
    auto operations = m_operations.find(typeid(T));
    if(operations == m_operations.end()) {
      m_operations.emplace_hint(operations, typeid(T), Operations(
        [] (const Selector& self, const Selector& selector) {
          return selector.get_type() == typeid(T) &&
            self.as<T>() == selector.as<T>();
        },
        [] (const Selector& self, const Stylist& base,
            const SelectionUpdateSignal& on_update) {
          return select(self.as<T>(), base, on_update);
        },
        [] (const Selector& self) {
          return std::hash<T>()(self.as<T>());
        }));
    }
  }

  template<typename U>
  const U& Selector::as() const {
    return std::any_cast<const U&>(m_selector);
  }

  template<typename F>
  decltype(auto) Selector::visit(F&& f) const {
    if constexpr(std::is_invocable_v<std::decay_t<F>>) {
      return std::forward<F>(f)();
    } else {
      using Parameter = typename TypeExtractor<
        Beam::GetFunctionParameters<std::decay_t<F>>>::type;
      if(m_selector.type() == typeid(Parameter)) {
        return std::forward<F>(f)(std::any_cast<const Parameter&>(m_selector));
      }
      throw std::bad_any_cast();
    }
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

namespace std {
  template<>
  struct hash<Spire::Styles::Selector> {
    std::size_t operator ()(const Spire::Styles::Selector& selector);
  };
}

#endif
