#ifndef SPIRE_STYLES_NOT_SELECTOR_HPP
#define SPIRE_STYLES_NOT_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget that isn't selected by a selector. */
  class NotSelector {
    public:

      /**
       * Constructs a NotSelector.
       * @param selector The selector to negate.
       */
      explicit NotSelector(Selector selector);

      /** Returns the selector being negated. */
      const Selector& get_selector() const;

      bool operator ==(const NotSelector& selector) const;

      bool operator !=(const NotSelector& selector) const;

    private:
      Selector m_selector;
  };

  /**
   * Provides an operator for negating a selector.
   * @param selector The selector to negate.
   */
  NotSelector operator !(Selector selector);

  SelectConnection select(const NotSelector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  std::unordered_set<Stylist*> select(
    const NotSelector& selector, std::unordered_set<Stylist*> sources);
}

#endif
