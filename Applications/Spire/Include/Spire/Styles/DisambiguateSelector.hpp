#ifndef SPIRE_STYLES_DISAMBIGUATE_SELECTOR_HPP
#define SPIRE_STYLES_DISAMBIGUATE_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Used to disambiguate between two types of selectors. */
  class DisambiguateSelector {
    public:

      /**
       * Constructs a DisambiguateSelector.
       * @param selector The selector to apply.
       */
      DisambiguateSelector(Selector selector);

      /** Returns the selector. */
      const Selector& get_selector() const;

      bool operator ==(const DisambiguateSelector& selector) const;

      bool operator !=(const DisambiguateSelector& selector) const;

    private:
      Selector m_selector;
  };

  /**
   * Provides an operator for the DisambiguateSelector.
   * @param selector The selector to apply.
   */
  DisambiguateSelector operator +(Selector base);

  std::vector<Stylist*> select(
    const DisambiguateSelector& selector, Stylist& source);

  std::vector<QWidget*> build_reach(
    const DisambiguateSelector& selector, QWidget& source);
}

#endif
