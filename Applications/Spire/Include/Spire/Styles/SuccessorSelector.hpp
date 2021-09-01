#ifndef SPIRE_STYLES_SUCCESSOR_SELECTOR_HPP
#define SPIRE_STYLES_SUCCESSOR_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects the first descendent. */
  class SuccessorSelector {
    public:

      /**
       * Constructs a SuccessorSelector.
       * @param base The selector for the base widget.
       * @param successor The selector for the successor.
       */
      SuccessorSelector(Selector base, Selector successor);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the successor selector. */
      const Selector& get_successor() const;

      bool operator ==(const SuccessorSelector& selector) const;

      bool operator !=(const SuccessorSelector& selector) const;

    private:
      Selector m_base;
      Selector m_successor;
  };

  /**
   * Provides an operator for the SuccessorSelector.
   * @param base The selector for the base widget.
   * @param successor The selector for the successor.
   */
  SuccessorSelector operator >(Selector base, Selector successor);

  SelectConnection select(const SuccessorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);
}

#endif
