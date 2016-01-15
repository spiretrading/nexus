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
      typedef InventoryType Inventory;

      //! The index used to look up inventories.
      typedef typename Inventory::Position::Index Index;

      //! The key used to lookup Inventories.
      typedef typename Inventory::Position::Key Key;
  };
}
}

#endif
