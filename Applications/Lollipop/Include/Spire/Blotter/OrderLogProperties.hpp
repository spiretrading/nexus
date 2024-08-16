#ifndef SPIRE_ORDERLOGPROPERTIES_HPP
#define SPIRE_ORDERLOGPROPERTIES_HPP
#include <Beam/Collections/EnumSet.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleBitset.hpp>
#include "Nexus/Definitions/OrderStatus.hpp"

namespace Spire {

  /*! \struct OrderLogProperties
      \brief Stores the properties used to display the Order log.
   */
  struct OrderLogProperties {

    /*! \enum OrderStatusFilterType
        \brief Specifies the type of OrderStatus filter used.
     */
    enum class OrderStatusFilterType {

      //! List all Orders.
      ALL_ORDERS,

      //! List only live Orders.
      LIVE_ORDERS,

      //! List only terminal Orders.
      TERMINAL_ORDERS,

      //! Use the custom OrderStatus filter.
      CUSTOM
    };

    //! Returns the default OrderLogProperties.
    static OrderLogProperties GetDefault();

    //! The type of OrderStatus filter used.
    OrderStatusFilterType m_orderStatusFilterType;

    //! Used to filter which OrderStatus to display.
    Beam::EnumSet<Nexus::OrderStatus> m_orderStatusFilter;
  };
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Spire::OrderLogProperties> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Spire::OrderLogProperties& value,
        unsigned int version) {
      shuttle.Shuttle("order_status_filter_type",
        value.m_orderStatusFilterType);
      shuttle.Shuttle("order_status_filter", value.m_orderStatusFilter);
    }
  };
}
}

#endif
