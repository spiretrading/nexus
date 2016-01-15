#ifndef NEXUS_ORDERTYPE_HPP
#define NEXUS_ORDERTYPE_HPP
#include <Beam/Collections/Enum.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /*! \enum OrderType
      \brief Represents the types of Orders.
   */
  BEAM_ENUM(OrderType,

    //! Market order.
    MARKET,

    //! Limit order.
    LIMIT,

    //! Pegged order.
    PEGGED,

    //! Stop order.
    STOP);

  //! Returns the single character FIX representation of an OrderType.
  inline char ToChar(OrderType value) {
    if(value == OrderType::MARKET) {
      return '1';
    } else if(value == OrderType::LIMIT) {
      return '2';
    } else if(value == OrderType::PEGGED) {
      return 'P';
    } else if(value == OrderType::STOP) {
      return '3';
    }
    BOOST_THROW_EXCEPTION(std::runtime_error("OrderType not found: " +
      boost::lexical_cast<std::string>(static_cast<int>(value))));
  }

  //! Returns the string representation of an OrderType.
  inline std::string ToString(OrderType value) {
    if(value == OrderType::MARKET) {
      return "MKT";
    } else if(value == OrderType::LIMIT) {
      return "LMT";
    } else if(value == OrderType::PEGGED) {
      return "PEG";
    } else if(value == OrderType::STOP) {
      return "STP";
    }
    BOOST_THROW_EXCEPTION(std::runtime_error("OrderType not found: " +
      boost::lexical_cast<std::string>(static_cast<int>(value))));
  }
}

#endif
