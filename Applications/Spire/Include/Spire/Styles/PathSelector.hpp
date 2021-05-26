#ifndef SPIRE_STYLES_PATH_SELECTOR_HPP
#define SPIRE_STYLES_PATH_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Applies two selectors in succession. */
  class PathSelector {
    public:

      /**
       * Constructs a PathSelector.
       * @param first The first selector to apply.
       * @param second The second selector to apply.
       */
      PathSelector(Selector first, Selector second);

      /** Returns the first selector applied. */
      const Selector& get_first() const;

      /** Returns the second selector applied. */
      const Selector& get_second() const;

      bool operator ==(const PathSelector& selector) const;

      bool operator !=(const PathSelector& selector) const;

    private:
      Selector m_first;
      Selector m_second;
  };

  /**
   * Provides an operator for the PathSelector.
   * @param first The first selector to apply.
   * @param second The second selector to apply.
   */
  template<typename T, typename U, typename = std::enable_if_t<
    std::is_constructible_v<Selector, const T&> &&
    std::is_constructible_v<Selector, const U&>>>
  auto operator /(T first, U second) {
    return PathSelector(std::move(first), std::move(second));
  }

  std::unordered_set<Stylist*> select(
    const PathSelector& selector, std::unordered_set<Stylist*> sources);

  std::vector<QWidget*> build_reach(
    const PathSelector& selector, QWidget& source);
}

#endif
