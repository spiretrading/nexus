#include "Spire/TimeAndSalesTester/TestTimeAndSalesModel.hpp"

using namespace boost::signals2;
using namespace Spire;

void TestTimeAndSalesModel::publish(const Details::TimeAndSalesEntry& entry) {
  m_entries.push(entry);
}

const std::deque<TestTimeAndSalesModel::LoadRequest>&
    TestTimeAndSalesModel::get_requests() const {
  return m_requests;
}

TestTimeAndSalesModel::LoadRequest TestTimeAndSalesModel::pop_request() {
  auto request = std::move(m_requests.front());
  m_requests.pop_front();
  return request;
}

QtPromise<void> TestTimeAndSalesModel::load_older(int max_count) {
  auto [future, promise] =
    make_future<std::vector<Details::TimeAndSalesEntry>>();
  m_requests.push_back(LoadRequest(max_count, std::move(future)));
  return std::move(promise).then([=] (auto&& result) {
    auto entries = std::move(result).get();
    m_entries.transact([&] {
      for(auto i = 0; i < std::ssize(entries); ++i) {
        m_entries.insert(entries[i], i);
      }
    });
  });
}

int TestTimeAndSalesModel::get_size() const {
  return m_entries.get_size();
}

const TestTimeAndSalesModel::Type&
    TestTimeAndSalesModel::get(int index) const {
  return m_entries.get(index);
}

connection TestTimeAndSalesModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_entries.connect_operation_signal(slot);
}

void TestTimeAndSalesModel::transact(
    const std::function<void ()>& transaction) {
  m_entries.transact([&] {
    transaction();
  });
}
