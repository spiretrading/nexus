#ifndef NEXUS_BOOKKEEPER_HPP
#define NEXUS_BOOKKEEPER_HPP
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Accounting/Inventory.hpp"

namespace Nexus::Accounting {

  /**
   * Performs bookkeeping and cost management of inventories.
   * @param <I> The type of Inventory to manage.
   */
  template<typename I>
  class Bookkeeper {
    public:

      /** The type of Inventory to manage. */
      using Inventory = I;

      /** The index used to look up inventories. */
      using Index = typename Inventory::Position::Index;

      /** The key used to lookup Inventories. */
      using Key = typename Inventory::Position::Key;
  };
}

#endif
