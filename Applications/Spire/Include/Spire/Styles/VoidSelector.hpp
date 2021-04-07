#ifndef SPIRE_STYLES_VOID_SELECTOR_HPP
#define SPIRE_STYLES_VOID_SELECTOR_HPP
#include <vector>
#include "Spire/Styles/Styles.hpp"

class QWidget;

namespace Spire::Styles {

  /** Selector that never matches anything. */
  class VoidSelector {
    public:
      bool is_match(const VoidSelector& selector) const;
  };

  std::vector<QWidget*> select(const VoidSelector& selector, QWidget& source);
}

#endif
