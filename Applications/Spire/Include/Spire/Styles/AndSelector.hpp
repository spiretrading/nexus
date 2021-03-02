#ifndef SPIRE_STYLES_AND_SELECTOR_HPP
#define SPIRE_STYLES_AND_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Implements a selector that evaluates to the logical conjunction of its
   * operands
   */
  class AndSelector {
    public:

      /**
       * Constructs an AndSelector as the conjunction of its operands.
       * @param left The left hand operand.
       * @param right The right hand operand.
       */
      explicit AndSelector(Selector left, Selector right);

      /** Returns the left hand selector. */
      const Selector& get_left() const;

      /** Returns the right hand selector. */
      const Selector& get_right() const;

    private:
      Selector m_left;
      Selector m_right;
  };

  /**
   * Provides an operator for the AndSelector.
   * @param left The left hand operand.
   * @param right The right hand operand.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator &&(T left, U right) {
    return AndSelector(std::move(left), std::move(right));
  }
}

#endif
