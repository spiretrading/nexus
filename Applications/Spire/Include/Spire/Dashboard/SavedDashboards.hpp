#ifndef SPIRE_SAVED_DASHBOARDS_HPP
#define SPIRE_SAVED_DASHBOARDS_HPP
#include <memory>
#include <string>
#include <vector>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardModelSchema.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /** Stores the user's list of saved dashboards. */
  class SavedDashboards {
    public:

      /** Stores a single saved dashboard. */
      struct Entry {

        /** The name of the dashboard. */
        std::string m_name;

        /** The dashboard's schema. */
        DashboardModelSchema m_schema;

        /** The settings used to display the dashboard. */
        std::shared_ptr<UI::WindowSettings> m_settings;

        /** Constructs an uninitialized Entry. */
        Entry() = default;

        /**
         * Constructs an Entry.
         * @param name The name of the dashboard.
         * @param schema The dashboard's schema.
         * @param settings The settings used to display the dashboard.
         */
        Entry(std::string name, DashboardModelSchema schema,
          std::shared_ptr<UI::WindowSettings> settings);
      };

      /**
       * Signals a dashboard was added.
       * @param entry The dashboard that was added.
       */
      using DashboardAddedSignal =
        boost::signals2::signal<void (const Entry& entry)>;

      /**
       * Signals a dashboard was removed.
       * @param entry The dashboard that was removed.
       */
      using DashboardRemovedSignal =
        boost::signals2::signal<void (const Entry& entry)>;

      /**
       * Loads the SavedDashboards from a UserProfile.
       * @param userProfile The UserProfile to load from.
       */
      static void Load(Beam::Out<UserProfile> userProfile);

      /**
       * Saves a UserProfile's SavedDashboards.
       * @param userProfile The UserProfile to save.
       */
      static void Save(const UserProfile& userProfile);

      /** Constructs an empty set of dashboards. */
      SavedDashboards() = default;

      /**
       * Copies the SavedDashboards.
       * @param savedDashboards The SavedDashboards to copy.
       */
      SavedDashboards(const SavedDashboards& savedDashboards);

      /**
       * Assigns the SavedDashboards.
       * @param savedDashboards The SavedDashboards to copy.
       */
      SavedDashboards& operator =(const SavedDashboards& savedDashboards);

      /** Returns the list of saved dashboards. */
      const std::vector<Entry>& GetDashboards() const;

      /**
       * Saves a dashboard.
       * @param entry The dashboard to save.
       */
      void Save(const Entry& entry);

      /**
       * Deletes a dashboard.
       * @param name The name of the dashboard to delete.
       */
      void Delete(const std::string& name);

      /**
       * Connects a slot to the DashboardAddedSignal.
       * @param slot The slot to connect.
       * @return A connection to the DashboardAddedSignal.
       */
      boost::signals2::connection ConnectDashboardAddedSignal(
        const DashboardAddedSignal::slot_type& slot) const;

      /**
       * Connects a slot to the DashboardRemovedSignal.
       * @param slot The slot to connect.
       * @return A connection to the DashboardRemovedSignal.
       */
      boost::signals2::connection ConnectDashboardRemovedSignal(
        const DashboardRemovedSignal::slot_type& slot) const;

    private:
      friend struct Beam::Serialization::Shuttle<Spire::SavedDashboards>;
      std::vector<Entry> m_dashboards;
      mutable DashboardAddedSignal m_dashboardAddedSignal;
      mutable DashboardRemovedSignal m_dashboardRemovedSignal;
  };
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Spire::SavedDashboards::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Spire::SavedDashboards::Entry& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("schema", value.m_schema);
      shuttle.Shuttle("settings", value.m_settings);
    }
  };

  template<>
  struct Shuttle<Spire::SavedDashboards> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Spire::SavedDashboards& value,
        unsigned int version) {
      shuttle.Shuttle("dashboards", value.m_dashboards);
    }
  };
}

#endif
