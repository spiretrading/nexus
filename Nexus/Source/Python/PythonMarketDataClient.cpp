#include "Nexus/Python/PythonMarketDataClient.hpp"
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonQueueWriter.hpp>

using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace std;

PythonMarketDataClient::PythonMarketDataClient(
    std::unique_ptr<VirtualMarketDataClient> client)
    : WrapperMarketDataClient<unique_ptr<VirtualMarketDataClient>>{
      std::move(client)} {}

PythonMarketDataClient::~PythonMarketDataClient() {
  GilRelease gil;
  boost::lock_guard<GilRelease> lock{gil};
  Close();
}

void PythonMarketDataClient::QueryOrderImbalances(
    const MarketWideDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryOrderImbalances(query, queue->GetSlot<OrderImbalance>());
}

void PythonMarketDataClient::QuerySequencedOrderImbalances(
    const MarketWideDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryOrderImbalances(query, queue->GetSlot<SequencedOrderImbalance>());
}

void PythonMarketDataClient::QueryBboQuotes(
    const SecurityMarketDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryBboQuotes(query, queue->GetSlot<BboQuote>());
}

void PythonMarketDataClient::QuerySequencedBboQuotes(
    const SecurityMarketDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryBboQuotes(query, queue->GetSlot<SequencedBboQuote>());
}

void PythonMarketDataClient::QueryBookQuotes(
    const SecurityMarketDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryBookQuotes(query, queue->GetSlot<BookQuote>());
}

void PythonMarketDataClient::QuerySequencedBookQuotes(
    const SecurityMarketDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryBookQuotes(query, queue->GetSlot<SequencedBookQuote>());
}

void PythonMarketDataClient::QueryMarketQuotes(
    const SecurityMarketDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryMarketQuotes(query, queue->GetSlot<MarketQuote>());
}

void PythonMarketDataClient::QuerySequencedMarketQuotes(
    const SecurityMarketDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryMarketQuotes(query, queue->GetSlot<SequencedMarketQuote>());
}

void PythonMarketDataClient::QueryTimeAndSales(
    const SecurityMarketDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryTimeAndSales(query, queue->GetSlot<TimeAndSale>());
}

void PythonMarketDataClient::QuerySequencedTimeAndSales(
    const SecurityMarketDataQuery& query,
    const std::shared_ptr<PythonQueueWriter>& queue) {
  WrapperMarketDataClient<std::unique_ptr<VirtualMarketDataClient>>::
    QueryTimeAndSales(query, queue->GetSlot<SequencedTimeAndSale>());
}
