#ifndef SPIRE_LEGACY_PROPERTY_HUB_MAP_HPP
#define SPIRE_LEGACY_PROPERTY_HUB_MAP_HPP
#include <memory>
#include <string>
#include <unordered_map>
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/PropertyHub.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Associates the identifiers used to link components before PropertyHubs were
   * introduced with the PropertyHubs that replace them.
   */
  class LegacyPropertyHubMap {
    public:

      /**
       * Signals that every reference to a PropertyHub is to be replaced.
       * @param source The PropertyHub being replaced.
       * @param destination The PropertyHub replacing it.
       */
      using MergeSignal =
        Signal<void (const std::shared_ptr<PropertyHub>& source,
          const std::shared_ptr<PropertyHub>& destination)>;

      /**
       * Returns the PropertyHub that a pair of linked identifiers belongs to,
       * constructing it if no such PropertyHub exists.
       * @param identifier The identifier of the component being restored.
       * @param link_identifier The identifier of the component that the
       *        component being restored is linked to.
       * @return The PropertyHub that both identifiers belong to.
       */
      std::shared_ptr<PropertyHub> acquire(
        const std::string& identifier, const std::string& link_identifier);

      /** Connects a slot to the MergeSignal. */
      boost::signals2::connection connect_merge_signal(
        const MergeSignal::slot_type& slot) const;

    private:
      mutable MergeSignal m_merge_signal;
      std::unordered_map<std::string, std::weak_ptr<PropertyHub>> m_hubs;

      std::shared_ptr<PropertyHub> find(const std::string& identifier) const;
      void merge(const std::shared_ptr<PropertyHub>& source,
        const std::shared_ptr<PropertyHub>& destination);
  };
}

#endif
