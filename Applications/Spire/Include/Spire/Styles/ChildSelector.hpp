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

    private:
      Selector m_base;
      Selector m_child;
  };

  /**
   * Provides an operator for the ChildSelector.
   * @param base The selector for the base widget.
   * @param child The selector for the child.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator >(T base, U child) {
    return ChildSelector(std::move(base), std::move(child));
  }
}

#endif
