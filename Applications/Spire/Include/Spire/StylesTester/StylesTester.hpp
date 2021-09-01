#ifndef SPIRE_STYLES_TESTER_HPP
#define SPIRE_STYLES_TESTER_HPP
#include <unordered_set>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles::Tests {

  /** Stores the arguments passed to a SelectionUpdateSignal. */
  struct SelectionUpdate {

    /** The additions passed to the update slot. */
    std::unordered_set<const Stylist*> m_additions;

    /** The removals passed to the update slot. */
    std::unordered_set<const Stylist*> m_removals;
  };
}

#endif
