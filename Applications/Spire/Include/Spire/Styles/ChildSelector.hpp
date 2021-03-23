#ifndef SPIRE_STYLES_CHILD_SELECTOR_HPP
#define SPIRE_STYLES_CHILD_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget if one of its children is selected. */
  class ChildSelector {
    public:

      /**
       * Constructs an ChildSelector.
       * @param base The selector for the base widget.
       * @param child The selector for the child.
       */
      explicit ChildSelector(Selector base, Selector child);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the child selector. */
      const Selector& get_child() const;

      bool is_match(const ChildSelector& selector) const;

    private:
      Selector m_base;
      Selector m_child;
  };

  /**
   * Provides an operator for the ChildSelector.
   * @param base The selector for the base widget.
   * @param child The selector for the child.
   */
  ChildSelector operator >(Selector base, Selector child);
}

#endif
