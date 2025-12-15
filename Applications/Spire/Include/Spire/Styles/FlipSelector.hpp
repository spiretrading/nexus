#ifndef SPIRE_STYLES_FLIP_SELECTOR_HPP
#define SPIRE_STYLES_FLIP_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"

namespace Spire::Styles {

  /** Used to flip what elements are selected. */
  class FlipSelector {
    public:

      /**
       * Constructs a FlipSelector.
       * @param selector The selector to apply.
       */
      explicit FlipSelector(Selector selector);

      /** Returns the selector. */
      const Selector& get_selector() const;

      bool operator ==(const FlipSelector& selector) const = default;

    private:
      Selector m_selector;
  };

  /**
   * Provides an operator for the FlipSelector.
   * @param selector The selector to apply.
   */
  FlipSelector operator +(Selector base);

  SelectConnection select(const FlipSelector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::FlipSelector> {
    std::size_t operator ()(
      const Spire::Styles::FlipSelector& selector) const noexcept;
  };
}

#endif
