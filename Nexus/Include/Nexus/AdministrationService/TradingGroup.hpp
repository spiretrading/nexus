#ifndef NEXUS_TRADING_GROUP_HPP
#define NEXUS_TRADING_GROUP_HPP
#include <utility>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>

namespace Nexus::AdministrationService {

  /** Represents a group of traders and their managers. */
  class TradingGroup {
    public:

      /** Constructs an uninitialized TradingGroup. */
      TradingGroup() = default;

      /**
       * Constructs a TradingGroup.
       * @param entry The DirectoryEntry containing the TradingGroup.
       * @param managers_directory The DirectoryEntry containing the managers.
       * @param managers The list of managers in the group.
       * @param traders_directory The DirectoryEntry containing the traders.
       * @param traders The list of traders in the group.
       */
      TradingGroup(Beam::ServiceLocator::DirectoryEntry entry,
        Beam::ServiceLocator::DirectoryEntry managers_directory,
        std::vector<Beam::ServiceLocator::DirectoryEntry> managers,
        Beam::ServiceLocator::DirectoryEntry traders_directory,
        std::vector<Beam::ServiceLocator::DirectoryEntry> traders) noexcept;

      /** Returns the DirectoryEntry representing this TradingGroup. */
      const Beam::ServiceLocator::DirectoryEntry& get_entry() const;

      /** Returns the DirectoryEntry for the managers directory. */
      const Beam::ServiceLocator::DirectoryEntry&
        get_managers_directory() const;

      /** Returns the list of managers in the group. */
      const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        get_managers() const;

      /** Returns the DirectoryEntry for the traders directory. */
      const Beam::ServiceLocator::DirectoryEntry& get_traders_directory() const;

      /** Returns the list of traders in the group. */
      const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        get_traders() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      Beam::ServiceLocator::DirectoryEntry m_entry;
      Beam::ServiceLocator::DirectoryEntry m_managers_directory;
      std::vector<Beam::ServiceLocator::DirectoryEntry> m_managers;
      Beam::ServiceLocator::DirectoryEntry m_traders_directory;
      std::vector<Beam::ServiceLocator::DirectoryEntry> m_traders;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline TradingGroup::TradingGroup(
    Beam::ServiceLocator::DirectoryEntry entry,
    Beam::ServiceLocator::DirectoryEntry managers_directory,
    std::vector<Beam::ServiceLocator::DirectoryEntry> managers,
    Beam::ServiceLocator::DirectoryEntry traders_directory,
    std::vector<Beam::ServiceLocator::DirectoryEntry> traders) noexcept
    : m_entry(std::move(entry)),
      m_managers_directory(std::move(managers_directory)),
      m_managers(std::move(managers)),
      m_traders_directory(std::move(traders_directory)),
      m_traders(std::move(traders)) {}

  inline const Beam::ServiceLocator::DirectoryEntry&
      TradingGroup::get_entry() const {
    return m_entry;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      TradingGroup::get_managers_directory() const {
    return m_managers_directory;
  }

  inline const std::vector<Beam::ServiceLocator::DirectoryEntry>&
      TradingGroup::get_managers() const {
    return m_managers;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      TradingGroup::get_traders_directory() const {
    return m_traders_directory;
  }

  inline const std::vector<Beam::ServiceLocator::DirectoryEntry>&
      TradingGroup::get_traders() const {
    return m_traders;
  }

  template<typename Shuttler>
  void TradingGroup::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("entry", m_entry);
    shuttle.Shuttle("managers_directory", m_managers_directory);
    shuttle.Shuttle("managers", m_managers);
    shuttle.Shuttle("traders_directory", m_traders_directory);
    shuttle.Shuttle("traders", m_traders);
  }
}

#endif
