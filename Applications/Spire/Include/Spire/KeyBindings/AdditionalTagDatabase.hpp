#ifndef SPIRE_ADDITIONAL_TAG_DATABASE_HPP
#define SPIRE_ADDITIONAL_TAG_DATABASE_HPP
#include <memory>
#include <unordered_map>
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/RegionMap.hpp"
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /**
   * Stores a database of additional tags with retrieval by region or
   * destination.
   */
  class AdditionalTagDatabase {
    public:

      /** Constructs an empty database. */
      AdditionalTagDatabase();

      /**
       * Adds a schema to this database.
       * @param region The region the schema applies to.
       * @param schema The schema to add.
       */
      void add(const Nexus::Region& region,
        const std::shared_ptr<AdditionalTagSchema>& schema);

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

    private:
      Nexus::RegionMap<std::unordered_map<
        int, std::shared_ptr<AdditionalTagSchema>>> m_schemas;
  };

  /** Returns a database of default additional tags. */
  const AdditionalTagDatabase& get_default_additional_tag_database();

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
