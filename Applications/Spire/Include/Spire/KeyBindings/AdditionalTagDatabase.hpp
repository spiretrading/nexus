#ifndef SPIRE_ADDITIONAL_TAG_DATABASE_HPP
#define SPIRE_ADDITIONAL_TAG_DATABASE_HPP
#include <memory>
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /**
   * Stores a database of additional tags with retrieval by region or
   * destination.
   */
  class AdditionalTagDatabase {
    public:

      /**
       * Returns the schema associated with a destination or <i>nullptr</i> iff
       * no schema exists.
       */
      const std::shared_ptr<AdditionalTagSchema>&
        find(const Nexus::Destination& destination, int key) const;

      /**
       * Returns the schema associated with a region and tag key or
       * <i>nullptr</i> iff no schema exists.
       */
      const std::shared_ptr<AdditionalTagSchema>&
        find(const Nexus::Region& region, int key) const;
  };

  /**
   * Returns the schema associated with a destination, and if no such schema is
   * found, then searches for the schema associated with a region. Returns
   * <i>nullptr</i> iff no schema exists.
   */
  const std::shared_ptr<AdditionalTagSchema>& find(
    const AdditionalTagDatabase& database,
    const Nexus::Destination& destination, const Nexus::Region& region,
    int key);
}

#endif
