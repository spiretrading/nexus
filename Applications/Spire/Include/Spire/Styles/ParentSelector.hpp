#ifndef SPIRE_STYLES_PARENT_SELECTOR_HPP
#define SPIRE_STYLES_PARENT_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

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
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator <(T base, U parent) {
    return ParentSelector(std::move(base), std::move(parent));
  }

  SelectConnection select(const ParentSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);
}

namespace std {
  template<>
  struct hash<Spire::Styles::ParentSelector> {
    std::size_t operator ()(
      const Spire::Styles::ParentSelector& selector) const;
  };
}

#endif
