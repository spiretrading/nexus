#ifndef SPIRE_STYLES_OR_SELECTOR_HPP
#define SPIRE_STYLES_OR_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"

namespace Spire::Styles {

  /** Selects a widget that is selected by at least one of two selectors. */
  class OrSelector {
    public:

      /**
       * Constructs an OrSelector.
       * @param left The left hand selector.
       * @param right The right hand selector.
       */
      OrSelector(Selector left, Selector right);

      /** Returns the left hand selector. */
      const Selector& get_left() const;

      /** Returns the right hand selector. */
      const Selector& get_right() const;

      bool operator ==(const OrSelector& selector) const = default;

    private:
      Selector m_left;
      Selector m_right;
  };

  /**
   * Provides an operator for the OrSelector.
   * @param left The left hand selector.
   * @param right The right hand selector.
   */
  OrSelector operator ||(Selector left, Selector right);

  SelectConnection select(const OrSelector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::OrSelector> {
    std::size_t operator ()(
      const Spire::Styles::OrSelector& selector) const noexcept;
  };
}

#endif
