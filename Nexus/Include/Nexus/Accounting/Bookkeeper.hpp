#ifndef NEXUS_BOOKKEEPER_HPP
#define NEXUS_BOOKKEEPER_HPP
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Accounting/Inventory.hpp"

namespace Nexus {
namespace Accounting {

  /*! \class Bookkeeper
      \brief Performs bookkeeping and cost management of inventories.
      \tparam InventoryType The type of Inventory to manage.
   */
  template<typename InventoryType>
  class Bookkeeper {
    public:

      //! The type of Inventory to manage.
      using Inventory = InventoryType;

      //! The index used to look up inventories.
      using Index = typename Inventory::Position::Index;

      //! The key used to lookup Inventories.
      using Key = typename Inventory::Position::Key;
  };
}
}

#endif
