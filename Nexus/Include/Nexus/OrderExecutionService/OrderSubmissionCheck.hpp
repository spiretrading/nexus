#ifndef NEXUS_ORDERSUBMISSIONCHECK_HPP
#define NEXUS_ORDERSUBMISSIONCHECK_HPP
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class OrderSubmissionCheck
      \brief Interface used to check if an Order submission is valid.
   */
  class OrderSubmissionCheck : private boost::noncopyable {
    public:
      virtual ~OrderSubmissionCheck() = default;

      //! Performs a check on a submission.
      /*!
        \param orderInfo The OrderInfo being submitted.
      */
      virtual void Submit(const OrderInfo& orderInfo) = 0;

      //! Adds an Order that successfully passed all submission checks.
      /*!
        \param order The successfully submitted Order.
      */
      virtual void Add(const Order& order);

      //! Indicates that a submission was rejected.
      /*!
        \param orderInfo The OrderInfo being rejected.
      */
      virtual void Reject(const OrderInfo& orderInfo);
  };

  inline void OrderSubmissionCheck::Add(const Order& order) {}

  inline void OrderSubmissionCheck::Reject(const OrderInfo& orderInfo) {}
}
}

#endif
