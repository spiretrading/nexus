#ifndef SPIRE_STYLES_STYLE_SHEET_HPP
#define SPIRE_STYLES_STYLE_SHEET_HPP
#include <functional>
#include <boost/optional/optional.hpp>
#include "Spire/Styles/Rule.hpp"

namespace Spire::Styles {

  /** Stores a list of styling rules for a widget. */
  class StyleSheet {
    public:

      /** Constructs an empty StyleSheet. */
      StyleSheet();

      /** Returns the list of style rules. */
      const std::vector<Rule>& get_rules() const;

      /**
       * Finds an existing rule that matches a selector.
       * @param selector The selector to match.
       * @return The rule matching the selector, if one exists.
       */
      boost::optional<const Rule&> find(const Selector& selector) const;

      /**
       * Returns the rule matching a selector, creating an empty Rule if one
       * does not yet exist.
       * @param selector The selector to match.
       * @return The rule matching the selector.
       */
      Rule& get(const Selector& selector);

      bool operator ==(const StyleSheet&) const = default;

    private:
      std::vector<Rule> m_rules;
  };
}

namespace std {
  template<>
  struct hash<Spire::Styles::StyleSheet> {
    std::size_t operator ()(
      const Spire::Styles::StyleSheet& styles) const noexcept;
  };
}

#endif
