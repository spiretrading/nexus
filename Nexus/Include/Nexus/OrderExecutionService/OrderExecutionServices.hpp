#ifndef NEXUS_ORDEREXECUTIONSERVICES_HPP
#define NEXUS_ORDEREXECUTIONSERVICES_HPP
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {
  using ExecutionReportQueryResult =
    Beam::Queries::QueryResult<SequencedExecutionReport>;
  using OrderSubmissionQueryResult =
    Beam::Queries::QueryResult<SequencedOrderRecord>;

  BEAM_DEFINE_SERVICES(OrderExecutionServices,

    /*! \interface Nexus::OrderExecutionService::NewOrderSingleService
        \brief Submits a request for a single Order.
        \param fields <code>OrderFields</code> The Order's fields.
        \return <code>SequencedAccountOrderInfo</code>
                The SequencedAccountOrderInfo representing the submitted Order.
    */
    //! \cond
    (NewOrderSingleService, "Nexus.OrderExecutionService.NewOrderSingleService",
      SequencedAccountOrderInfo, OrderFields, fields),
    //! \endcond

    /*! \interface Nexus::OrderExecutionService::UpdateOrderService
        \brief Updates an existing Order.
        \param order_id <code>OrderId</code> The id of the Order to update.
        \param execution_report <code>ExecutionReport</code> The ExecutionReport
               containing the update.
    */
    //! \cond
    (UpdateOrderService, "Nexus.OrderExecutionService.UpdateOrderService",
      void, OrderId, order_id, ExecutionReport, execution_report),
    //! \endcond

    /*! \interface Nexus::OrderExecutionService::QueryOrderSubmissionsService
        \brief Submits a query for an account's Order submissions.
        \param query <code>AccountQuery</code> The query to submit.
        \return <code>OrderSubmissionQueryResult</code> The list of Order
                submissions matching the query.
    */
    //! \cond
    (QueryOrderSubmissionsService,
      "Nexus.OrderExecutionService.QueryOrderSubmissionsService",
      OrderSubmissionQueryResult, AccountQuery, query),
    //! \endcond

    /*! \interface Nexus::OrderExecutionService::QueryExecutionReportsService
        \brief Submits a query for an account's ExecutionReports.
        \param query <code>AccountQuery</code> The query to submit.
        \return <code>ExecutionReportQueryResult</code> The list of
                ExecutionReports matching the query.
    */
    //! \cond
    (QueryExecutionReportsService,
      "Nexus.OrderExecutionService.QueryExecutionReportsService",
      ExecutionReportQueryResult, AccountQuery, query));
    //! \endcond

  BEAM_DEFINE_MESSAGES(OrderExecutionMessages,

    /*! \interface Nexus::OrderExecutionService::CancelOrderMessage
        \brief Submits a request to cancel an Order.
        \param id <code>OrderId</code> The id of the Order to cancel.
    */
    //! \cond
    (CancelOrderMessage, "Nexus.OrderExecutionService.CancelOrderMessage",
      OrderId, id),
    //! \endcond

    /*! \interface Nexus::OrderExecutionService::OrderSubmissionMessage
        \brief Sends the details of an Order submission.
        \param order <code>SequencedAccountOrderInfo</code>
               A SequencedAccountOrderRecord storing the details of the
               submission.
    */
    //! \cond
    (OrderSubmissionMessage,
      "Nexus.OrderExecutionService.OrderSubmissionMessage",
      SequencedAccountOrderRecord, order),
    //! \endcond

    /*! \interface Nexus::OrderExecutionService::OrderUpdateMessage
        \brief Sends an update for an Order.
        \param execution_report <code>ExecutionReport</code> The ExecutionReport
               containing the update.
    */
    //! \cond
    (OrderUpdateMessage, "Nexus.OrderExecutionService.OrderUpdateMessage",
      ExecutionReport, execution_report),

    /*! \interface Nexus::OrderExecutionService::ExecutionReportMessage
        \brief Sends a query's ExecutionReport.
        \param execution_report <code>SequencedAccountExecutionReport</code>
               The query's ExecutionReport.
    */
    //! \cond
    (ExecutionReportMessage,
      "Nexus.OrderExecutionService.ExecutionReportMessage",
      SequencedAccountExecutionReport, execution_report),
    //! \endcond

    /*! \interface Nexus::OrderExecutionService::EndOrderSubmissionQueryMessage
        \brief Terminates a previous query of an account's Order submissions.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account that was queried.
        \param id <code>int</code> The id of query to end.
    */
    //! \cond
    (EndOrderSubmissionQueryMessage,
      "Nexus.OrderExecutionService.EndOrderSubmissionQueryMessage",
      Beam::ServiceLocator::DirectoryEntry, account, int, id),
    //! \endcond

    /*! \interface Nexus::OrderExecutionService::EndExecutionReportQueryMessage
        \brief Terminates a previous query of an account's ExecutionReports.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account that was queried.
        \param id <code>int</code> The id of query to end.
    */
    //! \cond
    (EndExecutionReportQueryMessage,
      "Nexus.OrderExecutionService.EndExecutionReportQueryMessage",
      Beam::ServiceLocator::DirectoryEntry, account, int, id));
    //! \endcond
}
}

#endif
