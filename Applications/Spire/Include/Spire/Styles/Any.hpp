#ifndef SPIRE_STYLES_ANY_HPP
#define SPIRE_STYLES_ANY_HPP
#include <vector>
#include "Spire/Styles/Styles.hpp"

class QWidget;

namespace Spire::Styles {

  /** Selects all widgets unconditionally. */
  class Any {
    public:
      bool is_match(const Any& selector) const;
  };

  std::vector<QWidget*> select(const Any& selector, QWidget& source);
}

#endif
