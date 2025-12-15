#ifndef SPIRE_BLOTTER_SETTINGS_HPP
#define SPIRE_BLOTTER_SETTINGS_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterTaskProperties.hpp"
#include "Spire/Blotter/OrderLogProperties.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Stores settings related to blotters. */
  class BlotterSettings {
    public:

      /**
       * Signals a blotter was added.
       * @param blotter The blotter that was added.
       */
      using BlotterAddedSignal =
        boost::signals2::signal<void (BlotterModel& model)>;

      /**
       * Signals a blotter was removed.
       * @param blotter The blotter that was removed.
       */
      using BlotterRemovedSignal =
        boost::signals2::signal<void (BlotterModel& model)>;

      /**
       * Signals the active blotter was changed.
       * @param blotter The new active blotter.
       */
      using ActiveBlotterChangedSignal =
        boost::signals2::signal<void (BlotterModel& model)>;

      /**
       * Loads a UserProfile's BlotterSettings.
       * @param userProfile The UserProfile storing the BlotterSettings.
       */
      static void Load(Beam::Out<UserProfile> userProfile);

      /**
       * Saves a UserProfile's BlotterSettings.
       * @param userProfile The UserProfile whose BlotterSettings are to be
       *        saved.
       */
      static void Save(const UserProfile& userProfile);

      /**
       * Constructs a BlotterSettings.
       * @param userProfile The user's profile.
       */
      explicit BlotterSettings(Beam::Ref<UserProfile> userProfile);

      /** Returns all blotters. */
      const std::vector<std::unique_ptr<BlotterModel>>& GetAllBlotters() const;

      /** Returns the default BlotterTaskProperties. */
      const BlotterTaskProperties& GetDefaultBlotterTaskProperties() const;

      /** Returns the default OrderLogProperties. */
      const OrderLogProperties& GetDefaultOrderLogProperties() const;

      /**
       * Sets the default BlotterTaskProperties.
       * @param properties The default BlotterTaskProperties to use.
       */
      void SetDefaultBlotterTaskProperties(
        const BlotterTaskProperties& properties);

      /**
       * Sets the default OrderLogProperties.
       * @param properties The default OrderLogProperties to use.
       */
      void SetDefaultOrderLogProperties(const OrderLogProperties& properties);

      /**
       * Adds a blotter.
       * @param blotter The blotter to add.
       */
      void AddBlotter(std::unique_ptr<BlotterModel>&& blotter);

      /**
       * Removes a blotter.
       * @param blotter The blotter to remove.
       */
      void RemoveBlotter(const BlotterModel& blotter);

      /** Returns the user's own consolidated blotter. */
      const BlotterModel& GetConsolidatedBlotter() const;

      /** Returns the user's own consolidated blotter. */
      BlotterModel& GetConsolidatedBlotter();

      /**
       * Returns an account's consolidated blotter.
       * @param account The consolidated blotter's account.
       * @return The consolidated blotter for the specified <i>account</i>.
       */
      const BlotterModel& GetConsolidatedBlotter(
        const Beam::DirectoryEntry& account) const;

      /**
       * Returns an account's consolidated blotter.
       * @param account The consolidated blotter's account.
       * @return The consolidated blotter for the specified <i>account</i>.
       */
      BlotterModel& GetConsolidatedBlotter(
        const Beam::DirectoryEntry& account);

      /** Returns the active blotter model. */
      const BlotterModel& GetActiveBlotter() const;

      /** Returns the active blotter model. */
      BlotterModel& GetActiveBlotter();

      /**
       * Sets the active blotter.
       * @param blotter The blotter to use as the active blotter.
       */
      void SetActiveBlotter(const BlotterModel& blotter);

      /**
       * Adds a BlotterWindow to the list of recently closed windows.
       * @param window The BlotterWindow to add.
       */
      void AddRecentlyClosedWindow(const BlotterWindow& window);

      /**
       * Removes a BlotterWindow from the list of recently closed windows.
       * @param window The BlotterWindow to remove.
       */
      void RemoveRecentlyClosedWindow(const BlotterWindow& window);

      /**
       * Connects a slot to the BlotterAddedSignal.
       * @param slot The slot to connect.
       * @return The connection to the signal.
       */
      boost::signals2::connection ConnectBlotterAddedSignal(
        const BlotterAddedSignal::slot_type& slot);

      /**
       * Connects a slot to the BlotterRemovedSignal.
       * @param slot The slot to connect.
       * @return The connection to the signal.
       */
      boost::signals2::connection ConnectBlotterRemovedSignal(
        const BlotterRemovedSignal::slot_type& slot);

      /**
       * Connects a slot to the ActiveBlotterChangedSignal.
       * @param slot The slot to connect.
       * @return The connection to the signal.
       */
      boost::signals2::connection ConnectActiveBlotterChangedSignal(
        const ActiveBlotterChangedSignal::slot_type& slot);

    private:
      UserProfile* m_userProfile;
      BlotterTaskProperties m_defaultBlotterTaskProperties;
      OrderLogProperties m_defaultOrderLogProperties;
      std::vector<std::unique_ptr<BlotterModel>> m_blotters;
      std::unordered_map<const BlotterModel*, UI::WindowSettings*>
        m_recentlyClosedBlotters;
      mutable std::unordered_map<Beam::DirectoryEntry,
        BlotterModel*> m_consolidatedBlotters;
      BlotterModel* m_activeBlotter;
      BlotterAddedSignal m_blotterAddedSignal;
      BlotterRemovedSignal m_blotterRemovedSignal;
      ActiveBlotterChangedSignal m_activeBlotterChangedSignal;

      BlotterSettings(const BlotterSettings&) = delete;
      BlotterSettings& operator =(const BlotterSettings&) = delete;
  };
}

#endif
