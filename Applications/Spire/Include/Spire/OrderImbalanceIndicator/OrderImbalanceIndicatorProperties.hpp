#ifndef SPIRE_ORDERIMBALANCEINDICATORPROPERTIES_HPP
#define SPIRE_ORDERIMBALANCEINDICATORPROPERTIES_HPP
#include <unordered_set>
#include <Beam/Queries/Range.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleUnorderedSet.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class UserProfile;

  /*! \struct OrderImbalanceIndicatorProperties
      \brief Stores the properties used by an OrderImbalanceIndicatorModel.
   */
  struct OrderImbalanceIndicatorProperties {

    //! Returns the default properties.
    static OrderImbalanceIndicatorProperties GetDefault();

    //! Loads the OrderImbalanceIndicatorProperties from a UserProfile.
    /*!
      \param userProfile The UserProfile storing the properties.
    */
    static void Load(Beam::Out<UserProfile> userProfile);

    //! Saves a UserProfile's OrderImbalanceIndicatorProperties.
    /*!
      \param userProfile The UserProfile's properties to save.
    */
    static void Save(const UserProfile& userProfile);

    //! The query's start time.
    TimeRangeParameter m_startTime;

    //! The query's end time.
    TimeRangeParameter m_endTime;

    //! The set of filtered venues.
    std::unordered_set<Nexus::Venue> m_filteredVenues;

    //! Returns <code>true</code> iff a venue is filtered.
    bool IsDisplayed(Nexus::Venue venue) const;

    //! Returns <code>true</code> iff a venue is filtered.
    bool IsFiltered(Nexus::Venue venue) const;

    //! Returns the time range to query.
    /*!
      \param timeClient The time client used to compute times from offsets.
    */
    Beam::Range GetTimeRange(Beam::TimeClient& timeClient) const;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Spire::OrderImbalanceIndicatorProperties> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Spire::OrderImbalanceIndicatorProperties& value,
        unsigned int version) const {
      shuttle.shuttle("start_time", value.m_startTime);
      shuttle.shuttle("end_time", value.m_endTime);
      shuttle.shuttle("filtered_venues", value.m_filteredVenues);
    }
  };
}

#endif
