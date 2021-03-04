#ifndef SPIRE_STYLES_AND_SELECTOR_HPP
#define SPIRE_STYLES_AND_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget that is selected by two selectors. */
  class AndSelector {
    public:

      /**
       * Constructs an AndSelector.
       * @param left The left hand selector to match.
       * @param right The right hand selector to match.
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
   * @param left The left hand selector.
   * @param right The right hand selector.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator &&(T left, U right) {
    return AndSelector(std::move(left), std::move(right));
  }
}

#endif
