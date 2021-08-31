#ifndef SPIRE_STYLES_SELECTOR_HPP
#define SPIRE_STYLES_SELECTOR_HPP
#include <any>
#include <typeindex>
#include <type_traits>
#include <unordered_set>
#include <Beam/Utilities/Functional.hpp>
#include "Spire/Styles/Styles.hpp"

class QWidget;

namespace Spire::Styles {
  template<typename T, typename = void>
  struct is_selector_t : std::false_type {};

  template<typename T>
  struct is_selector_t<T, std::enable_if_t<std::is_same_v<decltype(
    select(std::declval<T>(), std::declval<std::unordered_set<Stylist*>>())),
    std::unordered_set<Stylist*>>>> : std::true_type {};

  template<typename T>
  constexpr auto is_selector_v = is_selector_t<T>::value;

  /**
   * The type of callable used to signal a change in the selected Stylists.
   * @param additions The set of Stylists added to the selection.
   * @param removals The set of Stylists removed from the selection.
   */
  using SelectionUpdate = std::function<
    void (const std::unordered_set<const Stylist*>& additions,
      const std::unordered_set<const Stylist*> removals)>;

  /** Stores a scoped connection to a select operation. */
  class SelectConnection {
    public:

      /** Constructs a stateless connection. */
      SelectConnection() = default;

      /**
       * Constructs a stateful connection.
       * @param args The arguments representing the state of the connection.
       */
      template<typename T, typename... U>
      explicit SelectConnection(T&& arg, U&&... args);

      SelectConnection(const SelectConnection&) = delete;

      SelectConnection(SelectConnection&&) = default;

    private:
      std::shared_ptr<void> m_state;
  };

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
      friend std::unordered_set<Stylist*>
        select(const Selector&, std::unordered_set<Stylist*>);
      friend SelectConnection select(
        const Selector&, const Stylist&, const SelectionUpdate&);
      friend std::unordered_set<QWidget*> build_reach(
        const Selector&, QWidget&);
      std::any m_selector;
      std::function<bool (const Selector&, const Selector&)> m_is_equal;
      std::function<std::unordered_set<Stylist*> (
        const Selector&, std::unordered_set<Stylist*>)> m_select;
      std::function<SelectConnection (
        const Selector&, const Stylist&, const SelectionUpdate&)>
        m_select_connection;
      std::function<std::unordered_set<QWidget*> (const Selector&, QWidget&)>
        m_reach;
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
    const SelectionUpdate& on_update);

  /**
   * Returns all Stylists that match a Selector.
   * @param selector The Selector to match.
   * @param sources The set if Stylists to match.
   * @return The set of of all Stylists that match the <i>selector<i>.
   */
  std::unordered_set<Stylist*> select(
    const Selector& selector, std::unordered_set<Stylist*> sources);

  /**
   * Returns all Stylists that match a Selector.
   * @param selector The Selector to match.
   * @param source The Stylist to match.
   * @return The set of of all Stylists that match the <i>selector<i>.
   */
  std::unordered_set<Stylist*> select(
    const Selector& selector, Stylist& source);

  /**
   * Returns the list of all widgets that could be selected by one of the rules
   * belonging to a StyleSheet.
   * @param style The StyleSheet to compute the list of.
   * @param source The widget that the <i>style</i> belongs to.
   * @return A list of all widgets that could be selected by the <i>style</i>.
   */
  std::unordered_set<QWidget*> build_reach(
    const StyleSheet& style, QWidget& source);

  /**
   * Returns the list of all widgets that could be selected by a Selector.
   * @param selector The Selector to compute the list of.
   * @param source The widget that the <i>selector</i> belongs to.
   * @return A list of all widgets that could be selected by the
   *         <i>selector</i>.
   */
  std::unordered_set<QWidget*> build_reach(
    const Selector& selector, QWidget& source);

  template<typename T, typename = std::enable_if_t<is_selector_v<T>>>
  std::unordered_set<QWidget*> build_reach(const T& selector, QWidget& source) {
    return {&source};
  }

  template<typename T, typename... U>
  SelectConnection::SelectConnection(T&& arg, U&&... args)
    : m_state(std::make_shared<
        std::tuple<std::remove_reference_t<T>, std::remove_reference_t<U>...>>(
          std::forward<T>(arg), std::forward<U>(args)...)) {}

  template<typename T, typename>
  Selector::Selector(T selector)
    : m_selector(std::move(selector)),
      m_is_equal([] (const Selector& self, const Selector& selector) {
        if(selector.get_type() != typeid(T)) {
          return false;
        }
        return self.as<T>() == selector.as<T>();
      }),
      m_select([] (const Selector& self, std::unordered_set<Stylist*> source) {
        return select(self.as<T>(), std::move(source));
      }),
      m_select_connection([] (const Selector& self, const Stylist& base,
            const SelectionUpdate& on_update) {
        return select(self.as<T>(), base, on_update);
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
