#include "Spire/TimeAndSalesTester/TestTimeAndSalesModel.hpp"

using namespace boost::signals2;
using namespace Spire;

void TestTimeAndSalesModel::publish(const Entry& entry) {
  m_update_signal(entry);
}

const std::deque<TestTimeAndSalesModel::QueryRequest>&
    TestTimeAndSalesModel::get_query_requests() const {
  return m_query_requests;
}

TestTimeAndSalesModel::QueryRequest TestTimeAndSalesModel::pop_query_request() {
  auto request = std::move(m_query_requests.front());
  m_query_requests.pop_front();
  return request;
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    TestTimeAndSalesModel::query_until(Beam::Sequence sequence, int max_count) {
  auto [future, promise] = make_future<std::vector<Entry>>();
  m_query_requests.push_back(
    QueryRequest(sequence, max_count, std::move(future)));
  return std::move(promise);
}

connection TestTimeAndSalesModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}
