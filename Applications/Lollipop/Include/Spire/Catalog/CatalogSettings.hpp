#ifndef SPIRE_CATALOG_SETTINGS_HPP
#define SPIRE_CATALOG_SETTINGS_HPP
#include <filesystem>
#include <map>
#include <vector>
#include <Beam/Pointers/Out.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/uuid/uuid.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Catalog/Catalog.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Stores the user's catalog related settings. */
  class CatalogSettings {
    public:

      /**
       * Signals a CatalogEntry was added.
       * @param entry The CatalogEntry that was added.
       */
      using CatalogEntryAddedSignal =
        boost::signals2::signal<void (CatalogEntry& entry)>;

      /**
       * Signals a CatalogEntry was removed.
       * @param entry The CatalogEntry that was removed.
       */
      using CatalogEntryRemovedSignal =
        boost::signals2::signal<void (CatalogEntry& entry)>;

      /**
       * Signals a CatalogTabModel was added.
       * @param tab The CatalogTabModel that was added.
       */
      using CatalogTabModelAddedSignal =
        boost::signals2::signal<void (CatalogTabModel& tab)>;

      /**
       * Signals a CatalogTabModel was removed.
       * @param tab The CatalogTabModel that was removed.
       */
      using CatalogTabModelRemovedSignal =
        boost::signals2::signal<void (CatalogTabModel& tab)>;

      /**
       * Loads a UserProfile's Catalog.
       * @param userProfile The UserProfile to load.
       */
      static void Load(Beam::Out<UserProfile> userProfile);

      /**
       * Saves a UserProfile's Catalog.
       * @param userProfile The UserProfile to save.
       */
      static void Save(const UserProfile& userProfile);

      /** Returns the path to the catalog library's registry. */
      static const std::string& GetCatalogLibraryRegistryPath();

      /**
       * Constructs a CatalogSettings.
       * @param settingsPath Path to where catalog related settings are stored.
       * @param hasRegistryAccess Whether write access to the registry is
       *        available.
       */
      CatalogSettings(
        const std::filesystem::path& settingsPath, bool hasRegistryAccess);

      ~CatalogSettings();

      /** Returns the path to the catalog settings. */
      const std::filesystem::path& GetSettingsPath() const;

      /**
       * Returns <code>true</code> iff write access to the registry is
       * available.
       */
      bool HasRegistryAccess() const;

      /**
       * Finds the CatalogEntry with a specified uid.
       * @param uid The uid of the CatalogEntry.
       * @return The CatalogEntry with the specified uid..
       */
      boost::optional<CatalogEntry&>
        FindEntry(const boost::uuids::uuid& uid) const;

      /**
       * Finds the CatalogEntry representing a CanvasNode.
       * @param node The CanvasNode to lookup.
       * @return The CatalogEntry representing the specified <i>node</i>.
       */
      boost::optional<CatalogEntry&> FindEntry(const CanvasNode& node) const;

      /** Returns the list of CatalogEntries. */
      const std::vector<std::unique_ptr<CatalogEntry>>&
        GetCatalogEntries() const;

      /** Returns the special All tab, containing all CatalogEntries. */
      const CatalogTabModel& GetAllTab() const;

      /** Returns the special All tab, containing all CatalogEntries. */
      CatalogTabModel& GetAllTab();

      /**
       * Adds a CatalogEntry.
       * @param entry The CatalogEntry to add.
       */
      void Add(std::unique_ptr<CatalogEntry>&& entry);

      /**
       * Removes a CatalogEntry.
       * @param entry The CatalogEntry to remove.
       */
      void Remove(const CatalogEntry& entry);

      /**
       * Replaces one CatalogEntry with another.
       * @param oldEntry The old CatalogEntry.
       * @param newEntry The new CatalogEntry taking the place of the old one.
       */
      void Replace(
        const CatalogEntry& oldEntry, std::unique_ptr<CatalogEntry>&& newEntry);

      /** Returns the list of CatalogTabModels. */
      const std::vector<std::unique_ptr<CatalogTabModel>>&
        GetCatalogTabs() const;

      /**
       * Adds a Catalog tab.
       * @param tab The model representing the Catalog tab to add.
       */
      void Add(std::unique_ptr<CatalogTabModel>&& tab);

      /**
       * Removes a Catalog tab.
       * @param tab The tab representing the Catalog tab to remove.
       */
      void Remove(const CatalogTabModel& tab);

      /**
       * Connects a slot to the CatalogEntryAddedSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectCatalogEntryAddedSignal(
        const CatalogEntryAddedSignal::slot_type& slot) const;

      /**
       * Connects a slot to the CatalogEntryRemovedSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectCatalogEntryRemovedSignal(
        const CatalogEntryRemovedSignal::slot_type& slot) const;

      /**
       * Connects a slot to the CatalogTabModelAddedSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectCatalogTabModelAddedSignal(
        const CatalogTabModelAddedSignal::slot_type& slot) const;

      /**
       * Connects a slot to the CatalogTabModelRemovedSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectCatalogTabModelRemovedSignal(
        const CatalogTabModelRemovedSignal::slot_type& slot) const;

    private:
      std::filesystem::path m_settingsPath;
      bool m_hasRegistryAccess;
      std::vector<std::unique_ptr<CatalogEntry>> m_catalogEntries;
      std::vector<std::unique_ptr<CatalogTabModel>> m_catalogTabs;
      std::map<boost::uuids::uuid, CatalogEntry*> m_uuidToEntry;
      mutable CatalogEntryAddedSignal m_catalogEntryAddedSignal;
      mutable CatalogEntryRemovedSignal m_catalogEntryRemovedSignal;
      mutable CatalogTabModelAddedSignal m_catalogTabModelAddedSignal;
      mutable CatalogTabModelRemovedSignal m_catalogTabModelRemovedSignal;

      CatalogSettings(const CatalogSettings&) = delete;
      CatalogSettings& operator =(const CatalogSettings&) = delete;
  };
}

#endif
