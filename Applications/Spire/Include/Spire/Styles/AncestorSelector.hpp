#ifndef SPIRE_STYLES_ANCESTOR_SELECTOR_HPP
#define SPIRE_STYLES_ANCESTOR_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects all matching ancestors. */
  class AncestorSelector {
    public:

      /**
       * Constructs an AncestorSelector.
       * @param base The selector for the base widget.
       * @param ancestor The selector for the ancestor.
       */
      AncestorSelector(Selector base, Selector ancestor);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the ancestor selector. */
      const Selector& get_ancestor() const;

      bool operator ==(const AncestorSelector& selector) const;

      bool operator !=(const AncestorSelector& selector) const;

    private:
      Selector m_base;
      Selector m_ancestor;
  };

  /**
   * Provides an operator for the AncestorSelector.
   * @param base The selector for the base widget.
   * @param ancestor The selector for the ancestor.
   */
  AncestorSelector operator <<(Selector base, Selector ancestor);

  SelectConnection select(const AncestorSelector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);

  std::unordered_set<Stylist*> select(
    const AncestorSelector& selector, std::unordered_set<Stylist*> sources);

  std::unordered_set<QWidget*> build_reach(
    const AncestorSelector& selector, QWidget& source);
}

#endif
