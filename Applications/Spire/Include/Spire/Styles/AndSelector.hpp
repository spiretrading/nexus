#ifndef SPIRE_STYLES_AND_SELECTOR_HPP
#define SPIRE_STYLES_AND_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"

namespace Spire::Styles {

  /** Selects a widget that is selected by two selectors. */
  class AndSelector {
    public:

      /**
       * Constructs an AndSelector.
       * @param left The left hand selector to match.
       * @param right The right hand selector to match.
       */
      AndSelector(Selector left, Selector right);

      /** Returns the left hand selector. */
      const Selector& get_left() const;

      /** Returns the right hand selector. */
      const Selector& get_right() const;

      bool operator ==(const AndSelector& selector) const = default;

    private:
      Selector m_left;
      Selector m_right;
  };

  /**
   * Provides an operator for the AndSelector.
   * @param left The left hand selector.
   * @param right The right hand selector.
   */
  AndSelector operator &&(Selector left, Selector right);

  SelectConnection select(const AndSelector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::AndSelector> {
    std::size_t operator ()(
      const Spire::Styles::AndSelector& selector) const noexcept;
  };
}

#endif
