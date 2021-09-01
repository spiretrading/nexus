#ifndef SPIRE_STYLES_DESCENDANT_SELECTOR_HPP
#define SPIRE_STYLES_DESCENDANT_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget if a descendant is selected. */
  class DescendantSelector {
    public:

      /**
       * Constructs a DescendantSelector.
       * @param base The selector for the base widget.
       * @param descendant The selector for the descendant.
       */
      DescendantSelector(Selector base, Selector descendant);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the descendant selector. */
      const Selector& get_descendant() const;

      bool operator ==(const DescendantSelector& selector) const;

      bool operator !=(const DescendantSelector& selector) const;

    private:
      Selector m_base;
      Selector m_descendant;
  };

  /**
   * Provides an operator for the DescendantSelector.
   * @param base The selector for the base widget.
   * @param descendant The selector for the descendant.
   */
  DescendantSelector operator >>(Selector base, Selector descendant);

  SelectConnection select(const DescendantSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);

  std::unordered_set<Stylist*> select(
    const DescendantSelector& selector, std::unordered_set<Stylist*> sources);

  std::unordered_set<QWidget*> build_reach(
    const DescendantSelector& selector, QWidget& source);
}

#endif
