#ifndef SPIRE_BLOTTER_REGISTRY_HPP
#define SPIRE_BLOTTER_REGISTRY_HPP
#include <filesystem>
#include "Spire/Blotter/BlotterController.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/LegacyBlotterSettings.hpp"

namespace Spire {
  class UserProfile;

  /** Owns blotter controllers and manages the active blotter. */
  class BlotterRegistry {
    public:

      /** The active blotter. */
      using ActiveModel = ValueModel<std::shared_ptr<BlotterModel>>;

      /** Stores persistent blotters and the active selection. */
      struct State {

        /** The saved blotters. */
        std::vector<BlotterSettings> m_blotters;

        /** The active blotter's ID, or nil for the default. */
        boost::uuids::uuid m_active;

        /** Constructs empty registry settings. */
        State() = default;

        /**
         * Converts legacy blotter settings.
         * @param settings The saved legacy configuration.
         */
        explicit State(const LegacyBlotterSettings& settings);

        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
      };

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

      /** Returns the persistent blotters and active selection. */
      State save_state() const;

      /**
       * Restores a newly initialized registry.
       * @param state The saved configuration.
       */
      void restore(const State& state);

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
      std::unique_ptr<FlattenController> m_flatten_controller;
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

  /**
   * Loads blotter settings, falling back to legacy settings or defaults.
   * @param path The profile directory.
   */
  BlotterRegistry::State load_blotter_settings(
    const std::filesystem::path& path);

  /**
   * Saves blotter settings.
   * @param state The saved configuration.
   * @param path The profile directory.
   */
  void save_blotter_settings(
    const BlotterRegistry::State& state, const std::filesystem::path& path);

  template<Beam::IsShuttle S>
  void BlotterRegistry::State::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("blotters", m_blotters);
    shuttle.shuttle("active", m_active);
  }
}

#endif
