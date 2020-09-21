#ifndef NEXUS_FIX_CONVERSIONS_HPP
#define NEXUS_FIX_CONVERSIONS_HPP
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional/optional.hpp>
#include <quickfix/FixFields.h>
#include <quickfix/FixValues.h>
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/FixUtilities/FixUtilities.hpp"

namespace Nexus::FixUtilities {

  /** Converts a ptime into its FIX representation. */
  inline FIX::DateTime GetTimestamp(const boost::posix_time::ptime& timestamp) {
    return FIX::DateTime(timestamp.date().year(),
      timestamp.date().month(), timestamp.date().day(),
      static_cast<int>(timestamp.time_of_day().hours()),
      static_cast<int>(timestamp.time_of_day().minutes()),
      static_cast<int>(timestamp.time_of_day().seconds()),
      static_cast<int>(1000 * (static_cast<double>(
      timestamp.time_of_day().fractional_seconds()) /
      boost::posix_time::time_duration::ticks_per_second())));
  }

  /** Converts a ptime from its FIX representation. */
  inline boost::posix_time::ptime GetTimestamp(const FIX::DateTime& timestamp) {
    return boost::posix_time::ptime(boost::gregorian::date(
      static_cast<unsigned short>(timestamp.getYear()),
      static_cast<unsigned short>(timestamp.getMonth()),
      static_cast<unsigned short>(timestamp.getDay())),
      boost::posix_time::time_duration(timestamp.getHour(),
      timestamp.getMinute(), timestamp.getSecond(),
      static_cast<int>(boost::posix_time::time_duration::ticks_per_second() /
      1000.0) * timestamp.getMillisecond()));
  }

  /** Converts a UtcTimestamp to a ptime. */
  inline boost::posix_time::ptime GetTimestamp(
      const FIX::UtcTimeStamp& timestamp) {
    return boost::posix_time::ptime(boost::gregorian::date(
      static_cast<unsigned short>(timestamp.getYear()),
      static_cast<unsigned short>(timestamp.getMonth()),
      static_cast<unsigned short>(timestamp.getDay())),
      boost::posix_time::time_duration(timestamp.getHour(),
      timestamp.getMinute(), timestamp.getSecond(),
      static_cast<int>(boost::posix_time::time_duration::ticks_per_second() /
      1000.0) * timestamp.getMillisecond()));
  }

  /** Converts a ptime into its FIX representation. */
  inline FIX::UtcTimeStamp GetUtcTimestamp(
      const boost::posix_time::ptime& timestamp) {
    return FIX::UtcTimeStamp(static_cast<int>(timestamp.time_of_day().hours()),
      static_cast<int>(timestamp.time_of_day().minutes()),
      static_cast<int>(timestamp.time_of_day().seconds()),
      static_cast<int>(1000 * (static_cast<double>(
      timestamp.time_of_day().fractional_seconds()) /
      boost::posix_time::time_duration::ticks_per_second())),
      timestamp.date().day(), timestamp.date().month(),
      timestamp.date().year());
  }

  /** Converts a ptime from its FIX representation. */
  inline boost::posix_time::ptime GetUtcTimestamp(
      const FIX::UtcTimeStamp& timestamp) {
    return boost::posix_time::ptime(boost::gregorian::date(
      static_cast<unsigned short>(timestamp.getYear()),
      static_cast<unsigned short>(timestamp.getMonth()),
      static_cast<unsigned short>(timestamp.getDay())),
      boost::posix_time::time_duration(timestamp.getHour(),
      timestamp.getMinute(), timestamp.getSecond(),
      static_cast<int>(boost::posix_time::time_duration::ticks_per_second() /
      1000.0) * timestamp.getMillisecond()));
  }

  /** Converts an OrderStatus to its FIX representation. */
  inline boost::optional<OrderStatus> GetOrderStatus(
      const FIX::OrdStatus& ordStatus) {
    if(ordStatus == FIX::OrdStatus_PENDING_NEW) {
      return OrderStatus(OrderStatus::PENDING_NEW);
    } else if(ordStatus == FIX::OrdStatus_REJECTED) {
      return OrderStatus(OrderStatus::REJECTED);
    } else if(ordStatus == FIX::OrdStatus_NEW) {
      return OrderStatus(OrderStatus::NEW);
    } else if(ordStatus == FIX::OrdStatus_PARTIALLY_FILLED) {
      return OrderStatus(OrderStatus::PARTIALLY_FILLED);
    } else if(ordStatus == FIX::OrdStatus_EXPIRED) {
      return OrderStatus(OrderStatus::EXPIRED);
    } else if(ordStatus == FIX::OrdStatus_CANCELED) {
      return OrderStatus(OrderStatus::CANCELED);
    } else if(ordStatus == FIX::OrdStatus_SUSPENDED) {
      return OrderStatus(OrderStatus::SUSPENDED);
    } else if(ordStatus == FIX::OrdStatus_STOPPED) {
      return OrderStatus(OrderStatus::STOPPED);
    } else if(ordStatus == FIX::OrdStatus_FILLED) {
      return OrderStatus(OrderStatus::FILLED);
    } else if(ordStatus == FIX::OrdStatus_DONE_FOR_DAY) {
      return OrderStatus(OrderStatus::DONE_FOR_DAY);
    } else if(ordStatus == FIX::OrdStatus_PENDING_CANCEL) {
      return OrderStatus(OrderStatus::PENDING_CANCEL);
    }
    return boost::none;
  }

  /** Converts an OrderStatus from its FIX representation. */
  inline boost::optional<FIX::OrdStatus> GetOrderStatus(
      OrderStatus orderStatus) {
    if(orderStatus == OrderStatus::PENDING_NEW) {
      return FIX::OrdStatus(FIX::OrdStatus_PENDING_NEW);
    } else if(orderStatus == OrderStatus::REJECTED) {
      return FIX::OrdStatus(FIX::OrdStatus_REJECTED);
    } else if(orderStatus == OrderStatus::NEW) {
      return FIX::OrdStatus(FIX::OrdStatus_NEW);
    } else if(orderStatus == OrderStatus::PARTIALLY_FILLED) {
      return FIX::OrdStatus(FIX::OrdStatus_PARTIALLY_FILLED);
    } else if(orderStatus == OrderStatus::EXPIRED) {
      return FIX::OrdStatus(FIX::OrdStatus_EXPIRED);
    } else if(orderStatus == OrderStatus::CANCELED) {
      return FIX::OrdStatus(FIX::OrdStatus_CANCELED);
    } else if(orderStatus == OrderStatus::SUSPENDED) {
      return FIX::OrdStatus(FIX::OrdStatus_SUSPENDED);
    } else if(orderStatus == OrderStatus::STOPPED) {
      return FIX::OrdStatus(FIX::OrdStatus_STOPPED);
    } else if(orderStatus == OrderStatus::FILLED) {
      return FIX::OrdStatus(FIX::OrdStatus_FILLED);
    } else if(orderStatus == OrderStatus::DONE_FOR_DAY) {
      return FIX::OrdStatus(FIX::OrdStatus_DONE_FOR_DAY);
    } else if(orderStatus == OrderStatus::PENDING_CANCEL) {
      return FIX::OrdStatus(FIX::OrdStatus_PENDING_CANCEL);
    }
    return boost::none;
  }

  /** Converts an OrderType to its FIX representation. */
  inline boost::optional<FIX::OrdType> GetOrderType(OrderType orderType) {
    if(orderType == OrderType::LIMIT) {
      return FIX::OrdType(FIX::OrdType_LIMIT);
    } else if(orderType == OrderType::MARKET) {
      return FIX::OrdType(FIX::OrdType_MARKET);
    } else if(orderType == OrderType::PEGGED) {
      return FIX::OrdType(FIX::OrdType_PEGGED);
    } else if(orderType == OrderType::STOP) {
      return FIX::OrdType(FIX::OrdType_STOP);
    }
    return boost::none;
  }

  /** Converts an OrderType from its FIX representation. */
  inline boost::optional<OrderType> GetOrderType(const FIX::OrdType& ordType) {
    if(ordType == FIX::OrdType_LIMIT) {
      return OrderType(OrderType::LIMIT);
    } else if(ordType == FIX::OrdType_MARKET) {
      return OrderType(OrderType::MARKET);
    } else if(ordType == FIX::OrdType_PEGGED) {
      return OrderType(OrderType::PEGGED);
    } else if(ordType == FIX::OrdType_STOP) {
      return OrderType(OrderType::STOP);
    }
    return boost::none;
  }

  /**
   * Converts a Side to its FIX representation.
   * @param shortingFlag Whether the ask side is a short.
   */
  inline boost::optional<FIX::Side> GetSide(Side side, bool shortingFlag) {
    if(side == Side::BID) {
      return FIX::Side(FIX::Side_BUY);
    } else if(side == Side::ASK) {
      if(shortingFlag) {
        return FIX::Side(FIX::Side_SELL_SHORT);
      } else {
        return FIX::Side(FIX::Side_SELL);
      }
    }
    return boost::none;
  }

  /** Converts a TimeInForce's Type to its FIX representation. */
  inline boost::optional<FIX::TimeInForce> GetTimeInForceType(
      TimeInForce::Type timeInForceType) {
    if(timeInForceType == TimeInForce::Type::DAY) {
      return FIX::TimeInForce(FIX::TimeInForce_DAY);
    } else if(timeInForceType == TimeInForce::Type::GTC) {
      return FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_CANCEL);
    } else if(timeInForceType == TimeInForce::Type::OPG) {
      return FIX::TimeInForce(FIX::TimeInForce_AT_THE_OPENING);
    } else if(timeInForceType == TimeInForce::Type::MOC) {
      return FIX::TimeInForce(FIX::TimeInForce_AT_THE_CLOSE);
    } else if(timeInForceType == TimeInForce::Type::IOC) {
      return FIX::TimeInForce(FIX::TimeInForce_IMMEDIATE_OR_CANCEL);
    } else if(timeInForceType == TimeInForce::Type::FOK) {
      return FIX::TimeInForce(FIX::TimeInForce_FILL_OR_KILL);
    } else if(timeInForceType == TimeInForce::Type::GTX) {
      return FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_CROSSING);
    } else if(timeInForceType == TimeInForce::Type::GTD) {
      return FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_DATE);
    }
    return boost::none;
  }
}

#endif
