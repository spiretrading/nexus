#ifndef NEXUS_ORDER_INFO_HPP
#define NEXUS_ORDER_INFO_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores details about an Order submission. */
  struct OrderInfo {

    /** The OrderFields used in the submission. */
    OrderFields m_fields;

    /** The account that submitted the Order. */
    Beam::DirectoryEntry m_submission_account;

    /** The Order's id. */
    OrderId m_id;

    /** Whether the Order is a short sale. */
    bool m_shorting_flag;

    /** The Order's timestamp. */
    boost::posix_time::ptime m_timestamp;

    /** Constructs an empty OrderInfo. */
    OrderInfo() noexcept;

    /**
     * Constructs an OrderInfo.
     * @param fields The OrderFields used in the submission.
     * @param submission_account The account that submitted the Order.
     * @param id The Order's id.
     * @param shorting_flag Whether the Order is a short sale.
     * @param timestamp The Order's timestamp.
     */
    OrderInfo(OrderFields fields, Beam::DirectoryEntry submission_account,
      OrderId id, bool shorting_flag,
      boost::posix_time::ptime timestamp) noexcept;

    /**
     * Constructs an OrderInfo whose submission account is the same as the
     * account that the Order is assigned to.
     * @param fields The OrderFields used in the submission.
     * @param id The Order's id.
     * @param shorting_flag Whether the Order is a short sale.
     * @param timestamp The Order's timestamp.
     */
    OrderInfo(OrderFields fields, OrderId id, bool shorting_flag,
      boost::posix_time::ptime timestamp) noexcept;

    /**
     * Constructs an OrderInfo whose submission account is the same as the
     * account that the Order is assigned to and whose shorting flag is false.
     * @param fields The OrderFields used in the submission.
     * @param id The Order's id.
     * @param timestamp The Order's timestamp.
     */
    OrderInfo(OrderFields fields, OrderId id,
      boost::posix_time::ptime timestamp) noexcept;

    bool operator ==(const OrderInfo&) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out, const OrderInfo& value) {
    return out << '(' << value.m_fields << ' ' << value.m_submission_account <<
      ' ' << value.m_id << ' ' << value.m_shorting_flag << ' ' <<
      value.m_timestamp << ')';
  }

  inline OrderInfo::OrderInfo() noexcept
    : m_id(static_cast<OrderId>(-1)),
      m_shorting_flag(false) {}

  inline OrderInfo::OrderInfo(OrderFields fields,
    Beam::DirectoryEntry submission_account, OrderId id, bool shorting_flag,
    boost::posix_time::ptime timestamp) noexcept
    : m_fields(std::move(fields)),
      m_submission_account(std::move(submission_account)),
      m_id(id),
      m_shorting_flag(shorting_flag),
      m_timestamp(timestamp) {}

  inline OrderInfo::OrderInfo(OrderFields fields, OrderId id,
    bool shorting_flag, boost::posix_time::ptime timestamp) noexcept
    : m_fields(std::move(fields)),
      m_submission_account(m_fields.m_account),
      m_id(id),
      m_shorting_flag(shorting_flag),
      m_timestamp(timestamp) {}

  inline OrderInfo::OrderInfo(OrderFields fields, OrderId id,
    boost::posix_time::ptime timestamp) noexcept
    : OrderInfo(fields, id, false, timestamp) {}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::OrderInfo> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::OrderInfo& value, unsigned int version) const {
      shuttle.shuttle("fields", value.m_fields);
      shuttle.shuttle("submission_account", value.m_submission_account);
      shuttle.shuttle("order_id", value.m_id);
      shuttle.shuttle("shorting_flag", value.m_shorting_flag);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
