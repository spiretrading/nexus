#ifndef NEXUS_ORDER_INFO_HPP
#define NEXUS_ORDER_INFO_HPP
#include <ostream>
#include <tuple>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::OrderExecutionService {

  /** Stores details about an Order submission. */
  struct OrderInfo {

    /** The OrderFields used in the submission. */
    OrderFields m_fields;

    /** The account that submitted the Order. */
    Beam::ServiceLocator::DirectoryEntry m_submissionAccount;

    /** The Order's id. */
    OrderId m_orderId;

    /** Whether the Order is a short sale. */
    bool m_shortingFlag;

    /** The Order's timestamp. */
    boost::posix_time::ptime m_timestamp;

    /** Constructs an empty OrderInfo. */
    OrderInfo();

    /**
     * Constructs an OrderInfo.
     * @param fields The OrderFields used in the submission.
     * @param submissionAccount The account that submitted the Order.
     * @param orderId The Order's id.
     * @param shortingFlag Whether the Order is a short sale.
     * @param timestamp The Order's timestamp.
     */
    OrderInfo(OrderFields fields,
      Beam::ServiceLocator::DirectoryEntry submissionAccount, OrderId orderId,
      bool shortingFlag, boost::posix_time::ptime timestamp);

    /**
     * Constructs an OrderInfo whose submission account is the same as the
     * account that the Order is assigned to.
     * @param fields The OrderFields used in the submission.
     * @param orderId The Order's id.
     * @param shortingFlag Whether the Order is a short sale.
     * @param timestamp The Order's timestamp.
     */
    OrderInfo(OrderFields fields, OrderId orderId, bool shortingFlag,
      boost::posix_time::ptime timestamp);

    /**
     * Constructs an OrderInfo whose submission account is the same as the
     * account that the Order is assigned to and whose shorting flag is false.
     * @param fields The OrderFields used in the submission.
     * @param orderId The Order's id.
     * @param shortingFlag Whether the Order is a short sale.
     * @param timestamp The Order's timestamp.
     */
    OrderInfo(OrderFields fields, OrderId orderId,
      boost::posix_time::ptime timestamp);
  };

  /**
   * Tests if two OrderInfos are equal.
   * @param lhs The left hand side of the equality.
   * @param rhs The right hand side of the equality.
   * @return <code>true</code> iff <code>lhs</code> is equal to
   *         <code>rhs</code>.
   */
  inline bool operator ==(const OrderInfo& lhs, const OrderInfo& rhs) {
    return std::tie(lhs.m_fields, lhs.m_submissionAccount, lhs.m_orderId,
      lhs.m_shortingFlag, lhs.m_timestamp) == std::tie(rhs.m_fields,
      rhs.m_submissionAccount, rhs.m_orderId, rhs.m_shortingFlag,
      rhs.m_timestamp);
  }

  /**
   * Tests if two OrderInfos are not equal.
   * @param lhs The left hand side of the inequality.
   * @param rhs The right hand side of the inequality.
   * @return <code>true</code> iff <i>lhs</i> is not equal to <i>rhs</i>.
   */
  inline bool operator !=(const OrderInfo& lhs, const OrderInfo& rhs) {
    return !(lhs == rhs);
  }

  inline std::ostream& operator <<(std::ostream& out, const OrderInfo& value) {
    return out << "(" << value.m_fields << " " << value.m_submissionAccount <<
      " " << value.m_orderId << " " << value.m_shortingFlag << " " <<
      value.m_timestamp << ")";
  }

  inline OrderInfo::OrderInfo()
    : m_orderId(static_cast<OrderId>(-1)),
      m_shortingFlag(false) {}

  inline OrderInfo::OrderInfo(OrderFields fields,
    Beam::ServiceLocator::DirectoryEntry submissionAccount, OrderId orderId,
    bool shortingFlag, boost::posix_time::ptime timestamp)
    : m_fields(std::move(fields)),
      m_submissionAccount(std::move(submissionAccount)),
      m_orderId(orderId),
      m_shortingFlag(shortingFlag),
      m_timestamp(timestamp) {}

  inline OrderInfo::OrderInfo(OrderFields fields, OrderId orderId,
    bool shortingFlag, boost::posix_time::ptime timestamp)
    : m_fields(std::move(fields)),
      m_submissionAccount(m_fields.m_account),
      m_orderId(orderId),
      m_shortingFlag(shortingFlag),
      m_timestamp(timestamp) {}

  inline OrderInfo::OrderInfo(OrderFields fields, OrderId orderId,
    boost::posix_time::ptime timestamp)
    : OrderInfo(fields, orderId, false, timestamp) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::OrderExecutionService::OrderInfo> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::OrderExecutionService::OrderInfo& value, unsigned int version) {
      shuttle.Shuttle("fields", value.m_fields);
      shuttle.Shuttle("submission_account", value.m_submissionAccount);
      shuttle.Shuttle("order_id", value.m_orderId);
      shuttle.Shuttle("shorting_flag", value.m_shortingFlag);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
