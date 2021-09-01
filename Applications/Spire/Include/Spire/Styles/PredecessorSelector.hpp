#ifndef SPIRE_STYLES_PARENT_SELECTOR_HPP
#define SPIRE_STYLES_PARENT_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects the first ancestor. */
  class PredecessorSelector {
    public:

      /**
       * Constructs a PredecessorSelector.
       * @param base The selector for the base widget.
       * @param predecessor The selector for the predecessor.
       */
      PredecessorSelector(Selector base, Selector predecessor);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the predecessor selector. */
      const Selector& get_predecessor() const;

      bool operator ==(const PredecessorSelector& selector) const;

      bool operator !=(const PredecessorSelector& selector) const;

    private:
      Selector m_base;
      Selector m_predecessor;
  };

  /**
   * Provides an operator for the PredecessorSelector.
   * @param base The selector for the base widget.
   * @param predecessor The selector for the predecessor.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator <(T base, U predecessor) {
    return PredecessorSelector(std::move(base), std::move(predecessor));
  }

  SelectConnection select(const PredecessorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update);
}

#endif
