#ifndef SPIRE_STYLES_TOP_SELECTOR_HPP
#define SPIRE_STYLES_TOP_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget's top-most descendant. */
  class TopSelector {
    public:

      /**
       * Constructs a TopSelector.
       * @param base The selector for the base widget.
       * @param descendant The selector for the descendant.
       */
      TopSelector(Selector base, Selector descendant);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the descendant selector. */
      const Selector& get_descendant() const;

      bool operator ==(const TopSelector&) const = default;

    private:
      Selector m_base;
      Selector m_descendant;
  };

  /**
   * Provides an operator for the TopSelector.
   * @param base The selector for the base widget.
   * @param child The selector for the child.
   */
  TopSelector operator >(Selector base, Selector child);

  SelectConnection select(const TopSelector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);
}

#endif
