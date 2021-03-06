#ifndef SPIRE_STYLES_ANCESTOR_SELECTOR_HPP
#define SPIRE_STYLES_ANCESTOR_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget if an ancestor is selected. */
  class AncestorSelector {
    public:

      /**
       * Constructs an AncestorSelector.
       * @param base The selector for the base widget.
       * @param ancestor The selector for the ancestor.
       */
      explicit AncestorSelector(Selector base, Selector ancestor);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the ancestor selector. */
      const Selector& get_ancestor() const;

    private:
      Selector m_base;
      Selector m_ancestor;
  };

  /**
   * Provides an operator for the AncestorSelector.
   * @param base The selector for the base widget.
   * @param ancestor The selector for the ancestor.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator <<(T base, U ancestor) {
    return AncestorSelector(std::move(base), std::move(ancestor));
  }
}

#endif
