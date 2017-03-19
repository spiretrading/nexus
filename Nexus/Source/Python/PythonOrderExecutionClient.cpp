#include "Nexus/Python/PythonOrderExecutionClient.hpp"
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonQueueWriter.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace std;

PythonOrderExecutionClient::PythonOrderExecutionClient(
    std::unique_ptr<VirtualOrderExecutionClient> client)
    : WrapperOrderExecutionClient<std::unique_ptr<
        VirtualOrderExecutionClient>>{std::move(client)} {}

PythonOrderExecutionClient::~PythonOrderExecutionClient() {
  GilRelease gil;
  boost::lock_guard<GilRelease> lock{gil};
  Close();
}

void PythonOrderExecutionClient::QueryOrderRecords(const AccountQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperOrderExecutionClient<unique_ptr<VirtualOrderExecutionClient>>::
    QueryOrderRecords(query, queue->GetSlot<OrderRecord>());
}

void PythonOrderExecutionClient::QuerySequencedOrderSubmissions(
    const AccountQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperOrderExecutionClient<unique_ptr<VirtualOrderExecutionClient>>::
    QueryOrderSubmissions(query, queue->GetSlot<SequencedOrder>());
}

void PythonOrderExecutionClient::QueryOrderSubmissions(
    const AccountQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperOrderExecutionClient<unique_ptr<VirtualOrderExecutionClient>>::
    QueryOrderSubmissions(query, queue->GetSlot<const Order*>());
}

void PythonOrderExecutionClient::QueryExecutionReports(
    const AccountQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperOrderExecutionClient<unique_ptr<VirtualOrderExecutionClient>>::
    QueryExecutionReports(query, queue->GetSlot<ExecutionReport>());
}
