#ifndef SPIRE_STYLES_DESCENDANT_SELECTOR_HPP
#define SPIRE_STYLES_DESCENDANT_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget if a descendant is selected. */
  class DescendantSelector {
    public:

      /**
       * Constructs a DescendantSelector.
       * @param base The selector for the base widget.
       * @param descendant The selector for the descendant.
       */
      explicit DescendantSelector(Selector base, Selector descendant);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the descendant selector. */
      const Selector& get_descendant() const;

    private:
      Selector m_base;
      Selector m_descendant;
  };

  /**
   * Provides an operator for the DescendantSelector.
   * @param base The selector for the base widget.
   * @param descendant The selector for the descendant.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator >>(T base, U descendant) {
    return DescendantSelector(std::move(base), std::move(descendant));
  }
}

#endif
