#ifndef SPIRE_STYLES_STYLE_SHEET_HPP
#define SPIRE_STYLES_STYLE_SHEET_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Styles/Rule.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {
  class StyleSheet {
    public:
      StyleSheet();

      boost::optional<const Rule&> find(const Selector& selector) const;

      Rule& get(const Selector& selector);

    private:
      std::vector<Rule> m_rules;
  };
}

#endif
