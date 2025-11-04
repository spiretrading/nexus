#ifndef NEXUS_ORDER_EXECUTION_SERVICES_HPP
#define NEXUS_ORDER_EXECUTION_SERVICES_HPP
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"

namespace Nexus {
  using ExecutionReportQueryResult =
    Beam::QueryResult<SequencedExecutionReport>;
  using OrderSubmissionQueryResult = Beam::QueryResult<SequencedOrderRecord>;

  /** Standard name for the order execution service. */
  inline const auto ORDER_EXECUTION_SERVICE_NAME =
    std::string("order_execution_service");

  BEAM_DEFINE_SERVICES(order_execution_services,

    /**
     * Submits a request for a single Order.
     * @param fields The Order's fields.
     * @return The SequencedAccountOrderInfo representing the submitted Order.
     */
    (NewOrderSingleService, "Nexus.OrderExecutionService.NewOrderSingleService",
      SequencedAccountOrderInfo, (OrderFields, fields)),

    /**
     * Updates an existing Order.
     * @param order_id The id of the Order to update.
     * @param execution_report The ExecutionReport containing the update.
     */
    (UpdateOrderService, "Nexus.OrderExecutionService.UpdateOrderService",
      void, (OrderId, order_id), (ExecutionReport, execution_report)),

    /**
     * Loads an Order from its id.
     * @param id The id of the Order to load.
     * @return The SequencedAccountOrderRecord with the specified id.
     */
    (LoadOrderByIdService, "Nexus.OrderExecutionService.LoadOrderByIdService",
      boost::optional<SequencedAccountOrderRecord>, (OrderId, id)),

    /**
     * Submits a query for an account's Order submissions.
     * @param query The query to submit.
     * @return The list of Order submissions matching the query.
     */
    (QueryOrderSubmissionsService,
      "Nexus.OrderExecutionService.QueryOrderSubmissionsService",
      OrderSubmissionQueryResult, (AccountQuery, query)),

    /**
     * Submits a query for an account's ExecutionReports.
     * @param query The query to submit.
     * @return The list of ExecutionReports matching the query.
     */
    (QueryExecutionReportsService,
      "Nexus.OrderExecutionService.QueryExecutionReportsService",
      ExecutionReportQueryResult, (AccountQuery, query)));

  BEAM_DEFINE_MESSAGES(order_execution_messages,

    /**
     * Submits a request to cancel an Order.
     * @param id The id of the Order to cancel.
     */
    (CancelOrderMessage, "Nexus.OrderExecutionService.CancelOrderMessage",
      (OrderId, id)),

    /**
     * Sends the details of an Order submission.
     * @param order A SequencedAccountOrderRecord storing the details of the
     *        submission.
     */
    (OrderSubmissionMessage,
      "Nexus.OrderExecutionService.OrderSubmissionMessage",
      (SequencedAccountOrderRecord, order)),

    /**
     * Sends an update for an Order.
     * @param execution_report The ExecutionReport containing the update.
     */
    (OrderUpdateMessage, "Nexus.OrderExecutionService.OrderUpdateMessage",
      (ExecutionReport, execution_report)),

    /**
     * Sends a query's ExecutionReport.
     * @param execution_report The query's ExecutionReport.
     */
    (ExecutionReportMessage,
      "Nexus.OrderExecutionService.ExecutionReportMessage",
      (SequencedAccountExecutionReport, execution_report)),

    /**
     * Terminates a previous query of an account's Order submissions.
     * @param account The account that was queried.
     * @param id The id of query to end.
     */
    (EndOrderSubmissionQueryMessage,
      "Nexus.OrderExecutionService.EndOrderSubmissionQueryMessage",
      (Beam::DirectoryEntry, account), (int, id)),

    /**
     * Terminates a previous query of an account's ExecutionReports.
     * @param account The account that was queried.
     * @param id <code>int</code> The id of query to end.
     */
    (EndExecutionReportQueryMessage,
      "Nexus.OrderExecutionService.EndExecutionReportQueryMessage",
      (Beam::DirectoryEntry, account), (int, id)));
}

#endif
