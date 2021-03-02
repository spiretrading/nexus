#ifndef SPIRE_STYLES_OR_SELECTOR_HPP
#define SPIRE_STYLES_OR_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Implements a selector that evaluates to the logical disjunction of its
   * operands
   */
  class OrSelector {
    public:

      /**
       * Constructs an OrSelector as the disjunction of its operands.
       * @param left The left hand operand.
       * @param right The right hand operand.
       */
      explicit OrSelector(Selector left, Selector right);

      /** Returns the left hand selector. */
      const Selector& get_left() const;

      /** Returns the right hand selector. */
      const Selector& get_right() const;

    private:
      Selector m_left;
      Selector m_right;
  };

  /**
   * Provides an operator for the OrSelector.
   * @param left The left hand operand.
   * @param right The right hand operand.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator ||(T left, U right) {
    return OrSelector(std::move(left), std::move(right));
  }
}

#endif
