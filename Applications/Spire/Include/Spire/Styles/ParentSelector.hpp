#ifndef SPIRE_STYLES_PARENT_SELECTOR_HPP
#define SPIRE_STYLES_PARENT_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"

namespace Spire::Styles {

  /** Selects a widget's parent. */
  class ParentSelector {
    public:

      /**
       * Constructs a ParentSelector.
       * @param base The selector for the base widget.
       * @param parent The selector for the parent.
       */
      ParentSelector(Selector base, Selector parent);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the parent selector. */
      const Selector& get_parent() const;

      bool operator ==(const ParentSelector& selector) const = default;

    private:
      Selector m_base;
      Selector m_parent;
  };

  /**
   * Provides an operator for the ParentSelector.
   * @param base The selector for the base widget.
   * @param parent The selector for the parent.
   */
  ParentSelector operator <(Selector base, Selector parent);

  SelectConnection select(const ParentSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::ParentSelector> {
    std::size_t operator ()(
      const Spire::Styles::ParentSelector& selector) const noexcept;
  };
}

#endif
