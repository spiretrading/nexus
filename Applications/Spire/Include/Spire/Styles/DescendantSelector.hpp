#ifndef SPIRE_STYLES_DESCENDANT_SELECTOR_HPP
#define SPIRE_STYLES_DESCENDANT_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget if both an ancestor and descendant are both selected. */
  class DescendantSelector {
    public:

      /**
       * Constructs a DescendantSelector.
       * @param ancestor The selector for the ancestor.
       * @param descendant The selector for the descendant.
       */
      explicit DescendantSelector(Selector ancestor, Selector descendant);

      /** Returns the ancestor selector. */
      const Selector& get_ancestor() const;

      /** Returns the descendant selector. */
      const Selector& get_descendant() const;

    private:
      Selector m_ancestor;
      Selector m_descendant;
  };

  /**
   * Provides an operator for the DescendantSelector.
   * @param ancestor The selector for the ancestor.
   * @param descendant The selector for the descendant.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator >>(T ancestor, U descendant) {
    return DescendantSelector(std::move(ancestor), std::move(descendant));
  }
}

#endif
