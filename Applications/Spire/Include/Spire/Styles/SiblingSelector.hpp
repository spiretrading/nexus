#ifndef SPIRE_STYLES_SIBLING_SELECTOR_HPP
#define SPIRE_STYLES_SIBLING_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"

namespace Spire::Styles {

  /** Selects a widget that is a sibling of another. */
  class SiblingSelector {
    public:

      /**
       * Constructs a SiblingSelector.
       * @param base The selector for the base widget.
       * @param sibling The selector for the sibling.
       */
      SiblingSelector(Selector base, Selector sibling);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the sibling selector. */
      const Selector& get_sibling() const;

      bool operator ==(const SiblingSelector& selector) const = default;

    private:
      Selector m_base;
      Selector m_sibling;
  };

  /**
   * Provides an operator for the SiblingSelector.
   * @param base The base selector.
   * @param sibling The sibling selector.
   */
  SiblingSelector operator %(Selector base, Selector sibling);

  SelectConnection select(const SiblingSelector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::SiblingSelector> {
    std::size_t operator ()(
      const Spire::Styles::SiblingSelector& selector) const noexcept;
  };
}

#endif
