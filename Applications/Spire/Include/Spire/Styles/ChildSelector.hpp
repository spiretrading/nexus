#ifndef SPIRE_STYLES_CHILD_SELECTOR_HPP
#define SPIRE_STYLES_CHILD_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects all of a widget's children. */
  class ChildSelector {
    public:

      /**
       * Constructs a ChildSelector.
       * @param base The selector for the base widget.
       * @param child The selector for the child.
       */
      ChildSelector(Selector base, Selector child);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the child selector. */
      const Selector& get_child() const;

      bool operator ==(const ChildSelector& selector) const = default;

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

  SelectConnection select(const ChildSelector& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::ChildSelector> {
    std::size_t operator ()(const Spire::Styles::ChildSelector& selector) const;
  };
}

#endif
