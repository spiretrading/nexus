#ifndef SPIRE_BLOTTER_REGISTRY_HPP
#define SPIRE_BLOTTER_REGISTRY_HPP
#include "Spire/Blotter/BlotterController.hpp"

namespace Spire {
  class UserProfile;

  /** Owns blotter controllers and manages the active blotter. */
  class BlotterRegistry {
    public:

      /** A list of blotters. */
      using BlottersModel = ListModel<std::shared_ptr<BlotterModel>>;

      /** The active blotter. */
      using ActiveModel = ValueModel<std::shared_ptr<BlotterModel>>;

      /**
       * Constructs a BlotterRegistry.
       * @param profile Supplies account services and preferences.
       */
      explicit BlotterRegistry(Beam::Ref<UserProfile> profile);

      ~BlotterRegistry();

      /** Returns all managed blotters. */
      const std::shared_ptr<const BlottersModel>& get_blotters() const;

      /** Returns the pinned or open blotters. */
      const std::shared_ptr<const BlottersModel>& get_available() const;

      /** Returns the pinned, non-consolidated blotters. */
      const std::shared_ptr<const BlottersModel>& get_pinned() const;

      /** Returns the active blotter model. */
      const std::shared_ptr<const ActiveModel>& get_active() const;

      /** Returns a managed blotter's controller. */
      BlotterController& get(const BlotterModel& blotter) const;

      /** Returns the user's consolidated blotter controller. */
      BlotterController& get_consolidated();

      /**
       * Returns an account's consolidated controller, creating it if needed.
       * @param account The executing account.
       */
      BlotterController& get_consolidated(const Beam::DirectoryEntry& account);

      /**
       * Creates a non-consolidated blotter and its controller.
       * @param name The requested name, made unique if necessary.
       * @param account The executing account.
       */
      BlotterController& create(
        QString name, const Beam::DirectoryEntry& account);

      /** Makes a managed blotter active. */
      void set_active(const BlotterModel& blotter);

      /** Closes all blotter windows. */
      void close();

    private:
      struct Entry;
      UserProfile* m_profile;
      std::shared_ptr<BlotterCollection> m_collection;
      std::shared_ptr<const BlottersModel> m_blotters;
      std::shared_ptr<BlottersModel> m_available;
      std::shared_ptr<const BlottersModel> m_read_only_available;
      std::shared_ptr<BlottersModel> m_pinned;
      std::shared_ptr<const BlottersModel> m_read_only_pinned;
      std::shared_ptr<ActiveModel> m_active;
      std::shared_ptr<const ActiveModel> m_read_only_active;
      std::shared_ptr<PositionKeyBindingsModel> m_position_bindings;
      std::unordered_map<const BlotterModel*, std::unique_ptr<Entry>> m_entries;
      bool m_is_updating_active;

      BlotterRegistry(const BlotterRegistry&) = delete;
      BlotterRegistry& operator =(const BlotterRegistry&) = delete;
      QString make_name(QString name) const;
      BlotterController& add(QString name,
        const Beam::DirectoryEntry& account, bool is_consolidated);
      void update_lists(const BlotterModel& blotter);
      void remove(const BlotterModel& blotter);
      void on_open(const BlotterModel& blotter, bool is_open);
      void on_pinned(const BlotterModel& blotter, bool is_pinned);
      void on_active(const BlotterModel& blotter, bool is_active);
  };
}

#endif
