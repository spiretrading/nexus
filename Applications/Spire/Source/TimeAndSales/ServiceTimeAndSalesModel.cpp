#include "Spire/TimeAndSales/ServiceTimeAndSalesModel.hpp"
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include "Spire/TimeAndSales/LoadTimeAndSales.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

ServiceTimeAndSalesModel::ServiceTimeAndSalesModel(
    Ticker ticker, MarketDataClient client)
    : m_ticker(std::move(ticker)),
      m_client(std::move(client)),
      m_offset(0) {
  auto query = make_real_time_query(m_ticker);
  query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_client.query(query, m_event_handler.get_slot<SequencedBboQuote>(
    std::bind_front(&ServiceTimeAndSalesModel::on_bbo, this)));
  m_client.query(query, m_event_handler.get_slot<SequencedTimeAndSale>(
    std::bind_front(&ServiceTimeAndSalesModel::on_time_and_sale, this)));
}

QtPromise<void> ServiceTimeAndSalesModel::load_older(int max_count) {
  auto sequence = [&] {
    if(get_size() == 0) {
      return Beam::Sequence::PRESENT;
    }
    return decrement(get(0).m_time_and_sale.get_sequence());
  }();
  return QtPromise([=, ticker = m_ticker, client = m_client] () mutable {
    auto result = std::vector<Type>();
    for(auto& time_and_sale : load_time_and_sales(client, ticker,
        Range(Beam::Sequence::FIRST, sequence),
        SnapshotLimit::from_tail(max_count))) {
      result.push_back(Type(std::move(time_and_sale), BboIndicator::UNKNOWN));
    }
    load_bbo_indicators(client, ticker, result, none);
    return result;
  }, LaunchPolicy::ASYNC).then([=] (auto&& result) {
    try {
      auto& snapshot = result.get();
      auto end = [&] {
        if(get_size() == 0) {
          return snapshot.end();
        }
        return std::ranges::lower_bound(snapshot,
          get(0).m_time_and_sale.get_sequence(),
          std::ranges::less(), [] (const auto& entry) {
            return entry.m_time_and_sale.get_sequence();
          });
      }();
      auto count = static_cast<int>(std::distance(snapshot.begin(), end));
      if(count != 0) {
        m_entries.insert(m_entries.begin(),
          std::make_move_iterator(snapshot.begin()),
          std::make_move_iterator(end));
        m_offset = count;
        m_transaction.transact([&] {
          while(m_offset != 0) {
            --m_offset;
            m_transaction.push(AddOperation(0));
          }
        });
      }
    } catch(const std::exception&) {
      if(m_offset != 0) {
        m_entries.erase(m_entries.begin(), m_entries.begin() + m_offset);
        m_offset = 0;
      }
    }
  });
}

int ServiceTimeAndSalesModel::get_size() const {
  return static_cast<int>(m_entries.size()) - m_offset;
}

const ServiceTimeAndSalesModel::Type&
    ServiceTimeAndSalesModel::get(int index) const {
  if(index < 0 || index >= get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_entries[index + m_offset];
}

connection ServiceTimeAndSalesModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void ServiceTimeAndSalesModel::transact(
    const std::function<void ()>& transaction) {
  m_transaction.transact(transaction);
}

void ServiceTimeAndSalesModel::on_bbo(const SequencedBboQuote& bbo) {
  m_bbo = bbo;
}

void ServiceTimeAndSalesModel::on_time_and_sale(
    const SequencedTimeAndSale& time_and_sale) {
  if(get_size() != 0 && time_and_sale.get_sequence() <=
      m_entries.back().m_time_and_sale.get_sequence()) {
    return;
  }
  m_entries.push_back(
    Type(time_and_sale, get_bbo_indicator(*m_bbo, *time_and_sale)));
  m_transaction.push(AddOperation(get_size() - 1));
}
