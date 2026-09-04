#include "Spire/BookView/TopMpidPriceListModel.hpp"
#include <boost/optional/optional.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TopMpidPriceListModel::TopMpidPriceListModel(
    std::shared_ptr<BookQuoteListModel> quotes)
    : m_quotes(std::move(quotes)) {
  for(auto i = 0; i != m_quotes->get_size(); ++i) {
    on_operation(AddOperation(i));
  }
  m_connection = m_quotes->connect_operation_signal(
    std::bind_front(&TopMpidPriceListModel::on_operation, this));
}

int TopMpidPriceListModel::get_size() const {
  return m_top_prices.get_size();
}

const TopMpidPriceListModel::Type& TopMpidPriceListModel::get(int index) const {
  return m_top_prices.get(index);
}

connection TopMpidPriceListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_top_prices.connect_operation_signal(slot);
}

void TopMpidPriceListModel::transact(
    const std::function<void ()>& transaction) {
  m_top_prices.transact([&] {
    transaction();
  });
}

int TopMpidPriceListModel::find_index(Venue venue) const {
  auto i = m_indexes.find(venue);
  if(i == m_indexes.end()) {
    return -1;
  }
  return i->second;
}

void TopMpidPriceListModel::on_operation(
    const BookQuoteListModel::Operation& operation) {
  visit(operation,
    [&] (const BookQuoteListModel::AddOperation& operation) {
      auto& quote = m_quotes->get(operation.m_index);
      if(!quote.m_is_primary_mpid) {
        return;
      }
      auto mpid_index = find_index(quote.m_venue);
      if(mpid_index == -1) {
        m_indexes[quote.m_venue] = m_top_prices.get_size();
        m_top_prices.push(TopMpidPrice(quote.m_venue, quote.m_quote.m_price));
      } else {
        auto& top_mpid = m_top_prices.get(mpid_index);
        auto direction = get_direction(quote.m_quote.m_side);
        if(direction * quote.m_quote.m_price >= direction * top_mpid.m_price) {
          m_top_prices.set(
            mpid_index, TopMpidPrice(quote.m_venue, quote.m_quote.m_price));
        }
      }
    },
    [&] (const BookQuoteListModel::PreRemoveOperation& operation) {
      m_removed_quote = m_quotes->get(operation.m_index);
    },
    [&] (const BookQuoteListModel::RemoveOperation& operation) {
      if(!m_removed_quote.m_is_primary_mpid) {
        return;
      }
      auto mpid_index = find_index(m_removed_quote.m_venue);
      auto venue = m_top_prices.get(mpid_index).m_venue;
      if(m_top_prices.get(mpid_index).m_price !=
          m_removed_quote.m_quote.m_price) {
        return;
      }
      auto top = optional<Money>();
      for(auto i = 0; i != m_quotes->get_size(); ++i) {
        auto& quote = m_quotes->get(i);
        if(quote.m_is_primary_mpid && quote.m_venue == venue) {
          auto direction = get_direction(quote.m_quote.m_side);
          if(!top || direction * quote.m_quote.m_price > direction * *top) {
            top = quote.m_quote.m_price;
          }
        }
      }
      if(top) {
        m_top_prices.set(mpid_index, TopMpidPrice(venue, *top));
      } else {
        m_indexes.erase(venue);
        for(auto& entry : m_indexes) {
          if(entry.second > mpid_index) {
            --entry.second;
          }
        }
        m_top_prices.remove(mpid_index);
      }
    });
}
