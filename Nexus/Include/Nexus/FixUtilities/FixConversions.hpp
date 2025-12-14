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

namespace Nexus {

  /** Converts a ptime into its FIX representation. */
  inline FIX::DateTime get_timestamp(boost::posix_time::ptime timestamp) {
    return FIX::DateTime(timestamp.date().year(), timestamp.date().month(),
      timestamp.date().day(), static_cast<int>(timestamp.time_of_day().hours()),
      static_cast<int>(timestamp.time_of_day().minutes()),
      static_cast<int>(timestamp.time_of_day().seconds()),
      static_cast<int>(1000 *
        (static_cast<double>(timestamp.time_of_day().fractional_seconds()) /
          boost::posix_time::time_duration::ticks_per_second())));
  }

  /** Converts a ptime from its FIX representation. */
  inline boost::posix_time::ptime get_timestamp(
      const FIX::DateTime& timestamp) {
    return boost::posix_time::ptime(boost::gregorian::date(
      static_cast<unsigned short>(timestamp.getYear()),
      static_cast<unsigned short>(timestamp.getMonth()),
      static_cast<unsigned short>(timestamp.getDay())),
      boost::posix_time::time_duration(
        timestamp.getHour(), timestamp.getMinute(), timestamp.getSecond(),
        static_cast<int>(boost::posix_time::time_duration::ticks_per_second() /
          1000.0) * timestamp.getMillisecond()));
  }

  /** Converts a UtcTimestamp to a ptime. */
  inline boost::posix_time::ptime get_timestamp(
      const FIX::UtcTimeStamp& timestamp) {
    return boost::posix_time::ptime(boost::gregorian::date(
      static_cast<unsigned short>(timestamp.getYear()),
      static_cast<unsigned short>(timestamp.getMonth()),
      static_cast<unsigned short>(timestamp.getDay())),
      boost::posix_time::time_duration(
        timestamp.getHour(), timestamp.getMinute(), timestamp.getSecond(),
        static_cast<int>(boost::posix_time::time_duration::ticks_per_second() /
          1000.0) * timestamp.getMillisecond()));
  }

  /** Converts a ptime into its FIX representation. */
  inline FIX::UtcTimeStamp get_utc_timestamp(
      const boost::posix_time::ptime& timestamp) {
    return FIX::UtcTimeStamp(static_cast<int>(timestamp.time_of_day().hours()),
      static_cast<int>(timestamp.time_of_day().minutes()),
      static_cast<int>(timestamp.time_of_day().seconds()),
      static_cast<int>(1000 *
        (static_cast<double>(timestamp.time_of_day().fractional_seconds()) /
          boost::posix_time::time_duration::ticks_per_second())),
      timestamp.date().day(), timestamp.date().month(),
      timestamp.date().year());
  }

  /** Converts a ptime from its FIX representation. */
  inline boost::posix_time::ptime get_utc_timestamp(
      const FIX::UtcTimeStamp& timestamp) {
    return boost::posix_time::ptime(boost::gregorian::date(
      static_cast<unsigned short>(timestamp.getYear()),
      static_cast<unsigned short>(timestamp.getMonth()),
      static_cast<unsigned short>(timestamp.getDay())),
      boost::posix_time::time_duration(
        timestamp.getHour(), timestamp.getMinute(), timestamp.getSecond(),
        static_cast<int>(boost::posix_time::time_duration::ticks_per_second() /
          1000.0) * timestamp.getMillisecond()));
  }

  /** Converts an OrderStatus to its FIX representation. */
  inline boost::optional<OrderStatus> get_order_status(
      const FIX::OrdStatus& status) {
    if(status == FIX::OrdStatus_PENDING_NEW) {
      return OrderStatus(OrderStatus::PENDING_NEW);
    } else if(status == FIX::OrdStatus_REJECTED) {
      return OrderStatus(OrderStatus::REJECTED);
    } else if(status == FIX::OrdStatus_NEW) {
      return OrderStatus(OrderStatus::NEW);
    } else if(status == FIX::OrdStatus_PARTIALLY_FILLED) {
      return OrderStatus(OrderStatus::PARTIALLY_FILLED);
    } else if(status == FIX::OrdStatus_EXPIRED) {
      return OrderStatus(OrderStatus::EXPIRED);
    } else if(status == FIX::OrdStatus_CANCELED) {
      return OrderStatus(OrderStatus::CANCELED);
    } else if(status == FIX::OrdStatus_SUSPENDED) {
      return OrderStatus(OrderStatus::SUSPENDED);
    } else if(status == FIX::OrdStatus_STOPPED) {
      return OrderStatus(OrderStatus::STOPPED);
    } else if(status == FIX::OrdStatus_FILLED) {
      return OrderStatus(OrderStatus::FILLED);
    } else if(status == FIX::OrdStatus_DONE_FOR_DAY) {
      return OrderStatus(OrderStatus::DONE_FOR_DAY);
    } else if(status == FIX::OrdStatus_PENDING_CANCEL) {
      return OrderStatus(OrderStatus::PENDING_CANCEL);
    }
    return boost::none;
  }

  /** Converts an OrderStatus from its FIX representation. */
  inline boost::optional<FIX::OrdStatus> get_order_status(OrderStatus status) {
    if(status == OrderStatus::PENDING_NEW) {
      return FIX::OrdStatus(FIX::OrdStatus_PENDING_NEW);
    } else if(status == OrderStatus::REJECTED) {
      return FIX::OrdStatus(FIX::OrdStatus_REJECTED);
    } else if(status == OrderStatus::NEW) {
      return FIX::OrdStatus(FIX::OrdStatus_NEW);
    } else if(status == OrderStatus::PARTIALLY_FILLED) {
      return FIX::OrdStatus(FIX::OrdStatus_PARTIALLY_FILLED);
    } else if(status == OrderStatus::EXPIRED) {
      return FIX::OrdStatus(FIX::OrdStatus_EXPIRED);
    } else if(status == OrderStatus::CANCELED) {
      return FIX::OrdStatus(FIX::OrdStatus_CANCELED);
    } else if(status == OrderStatus::SUSPENDED) {
      return FIX::OrdStatus(FIX::OrdStatus_SUSPENDED);
    } else if(status == OrderStatus::STOPPED) {
      return FIX::OrdStatus(FIX::OrdStatus_STOPPED);
    } else if(status == OrderStatus::FILLED) {
      return FIX::OrdStatus(FIX::OrdStatus_FILLED);
    } else if(status == OrderStatus::DONE_FOR_DAY) {
      return FIX::OrdStatus(FIX::OrdStatus_DONE_FOR_DAY);
    } else if(status == OrderStatus::PENDING_CANCEL) {
      return FIX::OrdStatus(FIX::OrdStatus_PENDING_CANCEL);
    }
    return boost::none;
  }

  /** Converts an OrderStatus from its FIX representation. */
  inline boost::optional<FIX::OrdStatus> get_order_status(
      OrderStatus::Type status) {
    return get_order_status(OrderStatus(status));
  }

  /** Converts an OrderType to its FIX representation. */
  inline boost::optional<FIX::OrdType> get_order_type(OrderType type) {
    if(type == OrderType::LIMIT) {
      return FIX::OrdType(FIX::OrdType_LIMIT);
    } else if(type == OrderType::MARKET) {
      return FIX::OrdType(FIX::OrdType_MARKET);
    } else if(type == OrderType::PEGGED) {
      return FIX::OrdType(FIX::OrdType_PEGGED);
    } else if(type == OrderType::STOP) {
      return FIX::OrdType(FIX::OrdType_STOP);
    }
    return boost::none;
  }

  /** Converts an OrderType to its FIX representation. */
  inline boost::optional<FIX::OrdType> get_order_type(OrderType::Type type) {
    return get_order_type(OrderType(type));
  }

  /** Converts an OrderType from its FIX representation. */
  inline boost::optional<OrderType> get_order_type(const FIX::OrdType& type) {
    if(type == FIX::OrdType_LIMIT) {
      return OrderType(OrderType::LIMIT);
    } else if(type == FIX::OrdType_MARKET) {
      return OrderType(OrderType::MARKET);
    } else if(type == FIX::OrdType_PEGGED) {
      return OrderType(OrderType::PEGGED);
    } else if(type == FIX::OrdType_STOP) {
      return OrderType(OrderType::STOP);
    }
    return boost::none;
  }

  /**
   * Converts a Side to its FIX representation.
   * @param shorting_flag Whether the ask side is a short.
   */
  inline boost::optional<FIX::Side> get_side(Side side, bool shorting_flag) {
    if(side == Side::BID) {
      return FIX::Side(FIX::Side_BUY);
    } else if(side == Side::ASK) {
      if(shorting_flag) {
        return FIX::Side(FIX::Side_SELL_SHORT);
      } else {
        return FIX::Side(FIX::Side_SELL);
      }
    }
    return boost::none;
  }

  /** Converts a TimeInForce's Type to its FIX representation. */
  inline boost::optional<FIX::TimeInForce> get_time_in_force_type(
      TimeInForce::Type type) {
    if(type == TimeInForce::Type::DAY) {
      return FIX::TimeInForce(FIX::TimeInForce_DAY);
    } else if(type == TimeInForce::Type::GTC) {
      return FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_CANCEL);
    } else if(type == TimeInForce::Type::OPG) {
      return FIX::TimeInForce(FIX::TimeInForce_AT_THE_OPENING);
    } else if(type == TimeInForce::Type::MOC) {
      return FIX::TimeInForce(FIX::TimeInForce_AT_THE_CLOSE);
    } else if(type == TimeInForce::Type::IOC) {
      return FIX::TimeInForce(FIX::TimeInForce_IMMEDIATE_OR_CANCEL);
    } else if(type == TimeInForce::Type::FOK) {
      return FIX::TimeInForce(FIX::TimeInForce_FILL_OR_KILL);
    } else if(type == TimeInForce::Type::GTX) {
      return FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_CROSSING);
    } else if(type == TimeInForce::Type::GTD) {
      return FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_DATE);
    }
    return boost::none;
  }

  /** Converts a TimeInForce's Type to its FIX representation. */
  inline boost::optional<FIX::TimeInForce> get_time_in_force_type(
      TimeInForce::Type::Type type) {
    return get_time_in_force_type(TimeInForce::Type(type));
  }
}

#endif
