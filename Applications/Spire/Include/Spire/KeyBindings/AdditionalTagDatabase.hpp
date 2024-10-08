#ifndef SPIRE_ADDITIONAL_TAG_DATABASE_HPP
#define SPIRE_ADDITIONAL_TAG_DATABASE_HPP
#include <memory>
#include <unordered_map>
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
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

      /**
       * Constructs an empty database.
       * @param markets The database of markets used to form regions.
       * @param destinations The database of destinations used to resolve the
       *        region they belong to.
       */
      AdditionalTagDatabase(
        Nexus::MarketDatabase markets, Nexus::DestinationDatabase destinations);

      /**
       * Adds a schema to this database.
       * @param destination The destination the schema applies to.
       * @param schema The schema to add.
       */
      void add(const Nexus::Destination& destination,
        const std::shared_ptr<AdditionalTagSchema>& schema);

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

      /** Returns a list of all schemas for a given destination. */
      std::vector<std::shared_ptr<AdditionalTagSchema>>
        find(const Nexus::Destination& destination) const;

      /** Returns a list of all schemas for a given region. */
      std::vector<std::shared_ptr<AdditionalTagSchema>>
        find(const Nexus::Region& region) const;

    private:
      Nexus::MarketDatabase m_markets;
      Nexus::DestinationDatabase m_destinations;
      Nexus::RegionMap<std::unordered_map<
        int, std::shared_ptr<AdditionalTagSchema>>> m_schemas;
      std::unordered_map<Nexus::Destination, std::unordered_map<
        int, std::shared_ptr<AdditionalTagSchema>>> m_destination_schemas;
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

  /** Returns the list of all schemas for a destination and region. */
  std::vector<std::shared_ptr<AdditionalTagSchema>> find(
    const AdditionalTagDatabase& database,
    const Nexus::Destination& destination, const Nexus::Region& region);
}

#endif
