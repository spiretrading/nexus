#ifndef NEXUS_TRADING_GROUP_HPP
#define NEXUS_TRADING_GROUP_HPP
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus::AdministrationService {

  /** Represents a group of traders and their managers. */
  class TradingGroup {
    public:

      /** Constructs an uninitialized TradingGroup. */
      TradingGroup() = default;

      /**
       * Constructs a TradingGroup.
       * @param entry The DirectoryEntry containing the TradingGroup.
       * @param managersDirectory The DirectoryEntry containing the managers.
       * @param managers The list of managers in the group.
       * @param tradersDirectory The DirectoryEntry containing the traders.
       * @param traders The list of traders in the group.
       */
      TradingGroup(const Beam::ServiceLocator::DirectoryEntry& entry,
        const Beam::ServiceLocator::DirectoryEntry& managersDirectory,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& managers,
        const Beam::ServiceLocator::DirectoryEntry& tradersDirectory,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& traders);

      /** Returns the DirectoryEntry representing this TradingGroup. */
      const Beam::ServiceLocator::DirectoryEntry& GetEntry() const;

      /** Returns the DirectoryEntry for the managers directory. */
      const Beam::ServiceLocator::DirectoryEntry& GetManagersDirectory() const;

      /** Returns the list of managers in the group. */
      const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        GetManagers() const;

      /** Returns the DirectoryEntry for the traders directory. */
      const Beam::ServiceLocator::DirectoryEntry& GetTradersDirectory() const;

      /** Returns the list of traders in the group. */
      const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        GetTraders() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      Beam::ServiceLocator::DirectoryEntry m_entry;
      Beam::ServiceLocator::DirectoryEntry m_managersDirectory;
      std::vector<Beam::ServiceLocator::DirectoryEntry> m_managers;
      Beam::ServiceLocator::DirectoryEntry m_tradersDirectory;
      std::vector<Beam::ServiceLocator::DirectoryEntry> m_traders;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline TradingGroup::TradingGroup(
    const Beam::ServiceLocator::DirectoryEntry& entry,
    const Beam::ServiceLocator::DirectoryEntry& managersDirectory,
    const std::vector<Beam::ServiceLocator::DirectoryEntry>& managers,
    const Beam::ServiceLocator::DirectoryEntry& tradersDirectory,
    const std::vector<Beam::ServiceLocator::DirectoryEntry>& traders)
    : m_entry(entry),
      m_managersDirectory(managersDirectory),
      m_managers(managers),
      m_tradersDirectory(tradersDirectory),
      m_traders(traders) {}

  inline const Beam::ServiceLocator::DirectoryEntry&
      TradingGroup::GetEntry() const {
    return m_entry;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      TradingGroup::GetManagersDirectory() const {
    return m_managersDirectory;
  }

  inline const std::vector<Beam::ServiceLocator::DirectoryEntry>&
      TradingGroup::GetManagers() const {
    return m_managers;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      TradingGroup::GetTradersDirectory() const {
    return m_tradersDirectory;
  }

  inline const std::vector<Beam::ServiceLocator::DirectoryEntry>&
      TradingGroup::GetTraders() const {
    return m_traders;
  }

  template<typename Shuttler>
  void TradingGroup::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("entry", m_entry);
    shuttle.Shuttle("managers_directory", m_managersDirectory);
    shuttle.Shuttle("managers", m_managers);
    shuttle.Shuttle("traders_directory", m_tradersDirectory);
    shuttle.Shuttle("traders", m_traders);
  }
}

#endif
