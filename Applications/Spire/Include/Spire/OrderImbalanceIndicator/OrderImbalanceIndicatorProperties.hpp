#ifndef SPIRE_ORDERIMBALANCEINDICATORPROPERTIES_HPP
#define SPIRE_ORDERIMBALANCEINDICATORPROPERTIES_HPP
#include <unordered_set>
#include <Beam/Queries/Range.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleUnorderedSet.hpp>
#include <Beam/TimeService/TimeService.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

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

    //! The set of filtered markets.
    std::unordered_set<Nexus::MarketCode> m_filteredMarkets;

    //! Returns <code>true</code> iff a market is filtered.
    bool IsDisplayed(Nexus::MarketCode market) const;

    //! Returns <code>true</code> iff a market is filtered.
    bool IsFiltered(Nexus::MarketCode market) const;

    //! Returns the time range to query.
    /*!
      \param timeClient The time client used to compute times from offsets.
    */
    Beam::Queries::Range GetTimeRange(
      Beam::TimeService::TimeClientBox& timeClient) const;
  };
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Spire::OrderImbalanceIndicatorProperties> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Spire::OrderImbalanceIndicatorProperties& value, unsigned int version) {
      shuttle.Shuttle("start_time", value.m_startTime);
      shuttle.Shuttle("end_time", value.m_endTime);
      shuttle.Shuttle("filtered_markets", value.m_filteredMarkets);
    }
  };
}
}

#endif
