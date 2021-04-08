#ifndef SPIRE_STYLES_SELECTOR_HPP
#define SPIRE_STYLES_SELECTOR_HPP
#include <any>
#include <typeindex>
#include <type_traits>
#include <vector>
#include <Beam/Utilities/Functional.hpp>
#include "Spire/Styles/Styles.hpp"

class QWidget;

namespace Spire::Styles {
  template<typename T, typename = void>
  struct is_selector_t : std::false_type {};

  template<typename T>
  struct is_selector_t<T, std::enable_if_t<std::is_same_v<
    decltype(std::declval<T>().is_match(std::declval<T>())), bool>>> :
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
      friend std::vector<QWidget*> select(const Selector&, QWidget&);
      friend std::vector<QWidget*> build_reach(const Selector&, QWidget&);
      std::any m_selector;
      std::function<bool (const Selector&, const Selector&)> m_matcher;
      std::function<std::vector<QWidget*> (const Selector&, QWidget&)> m_select;
      std::function<std::vector<QWidget*> (const Selector&, QWidget&)> m_reach;
  };

  /**
   * Returns all selected widgets relative to a source.
   * @param selector The Selector to use.
   * @param source The QWidget used as the reference point for selection.
   *        This is usually the StyledWidget that the <i>selector</i> belongs
   *        to.
   * @return The list of all widgets that were selected.
   */
  std::vector<QWidget*> select(const Selector& selector, QWidget& source);

  template<typename T, typename = std::enable_if_t<is_selector_v<T>>>
  std::vector<QWidget*> select(const T& selector, QWidget& source) {
    static_assert(
      false, "All selector types must implement a select function.");
  }

  /**
   * Returns the list of all widgets that could be selected by one of the rules
   * belonging to a StyleSheet.
   * @param style The StyleSheet to compute the list of.
   * @param source The widget that the <i>style</i> belongs to.
   * @return A list of all widgets that could be selected by the <i>style</i>.
   */
  std::vector<QWidget*> build_reach(const StyleSheet& style, QWidget& source);

  /**
   * Returns the list of all widgets that could be selected by a Selector.
   * @param selector The Selector to compute the list of.
   * @param source The widget that the <i>selector</i> belongs to.
   * @return A list of all widgets that could be selected by the
   *         <i>selector</i>.
   */
  std::vector<QWidget*> build_reach(const Selector& selector, QWidget& source);

  template<typename T, typename = std::enable_if_t<is_selector_v<T>>>
  std::vector<QWidget*> build_reach(const T& selector, QWidget& source) {
    return {&source};
  }

  template<typename T, typename>
  Selector::Selector(T selector)
    : m_selector(std::move(selector)),
      m_matcher([] (const Selector& self, const Selector& selector) {
        if(selector.get_type() != typeid(T)) {
          return false;
        }
        auto& left = self.as<T>();
        auto& right = selector.as<T>();
        return left.is_match(right);
      }),
      m_select([] (const Selector& self, QWidget& widget) {
        return select(self.as<T>(), widget);
      }),
      m_reach([] (const Selector& self, QWidget& widget) {
        return build_reach(self.as<T>(), widget);
      }) {}

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

#endif
