#ifndef SPIRE_STYLES_ANCESTOR_SELECTOR_HPP
#define SPIRE_STYLES_ANCESTOR_SELECTOR_HPP
#include <utility>
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
      explicit AncestorSelector(Selector base, Selector ancestor);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the ancestor selector. */
      const Selector& get_ancestor() const;

      bool is_match(const AncestorSelector& selector) const;

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

  std::vector<QWidget*> select(
    const AncestorSelector& selector, QWidget& source);
}

#endif
