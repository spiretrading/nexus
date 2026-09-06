#include "Spire/BookView/TopMpidPriceListModel.hpp"
#include <boost/optional/optional.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  bool is_top_price_equivalent(
      const BookQuote& previous, const BookQuote& value) {
    return previous.m_is_primary_mpid == value.m_is_primary_mpid &&
      previous.m_venue == value.m_venue &&
      previous.m_quote.m_side == value.m_quote.m_side &&
      previous.m_quote.m_price == value.m_quote.m_price;
  }
}

TopMpidPriceListModel::TopMpidPriceListModel(
    std::shared_ptr<BookQuoteListModel> quotes)
    : m_quotes(std::move(quotes)) {
  for(auto i = 0; i != m_quotes->get_size(); ++i) {
    on_operation(AddOperation(i));
  }
  m_connection = m_quotes->connect_operation_signal(
    std::bind_front(&TopMpidPriceListModel::on_operation, this));
}

std::shared_ptr<TopMpidPriceListModel::TopPriceModel>
    TopMpidPriceListModel::get_top_price(Venue venue) {
  auto& model = m_top_price_models[venue];
  if(!model) {
    model =
      std::make_shared<LocalValueModel<optional<Money>>>(find_price(venue));
  }
  return model;
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

optional<int> TopMpidPriceListModel::find_index(Venue venue) const {
  auto i = m_indexes.find(venue);
  if(i == m_indexes.end()) {
    return none;
  }
  return i->second;
}

optional<Money> TopMpidPriceListModel::find_price(Venue venue) const {
  if(auto index = find_index(venue)) {
    return m_top_prices.get(*index).m_price;
  }
  return none;
}

void TopMpidPriceListModel::update_top_price_model(Venue venue) {
  auto i = m_top_price_models.find(venue);
  if(i == m_top_price_models.end()) {
    return;
  }
  auto price = find_price(venue);
  if(i->second->get() != price) {
    i->second->set(price);
  }
}

void TopMpidPriceListModel::update_top_price(Venue venue, Side side) {
  auto prices = m_prices.find(venue);
  auto venue_index = find_index(venue);
  if(prices == m_prices.end()) {
    if(!venue_index) {
      return;
    }
    m_indexes.erase(venue);
    for(auto& entry : m_indexes) {
      if(entry.second > *venue_index) {
        --entry.second;
      }
    }
    m_top_prices.remove(*venue_index);
  } else {
    auto top =
      pick(side, prices->second.begin()->first, prices->second.rbegin()->first);
    if(!venue_index) {
      m_indexes[venue] = m_top_prices.get_size();
      m_top_prices.push(TopMpidPrice(venue, top));
    } else if(m_top_prices.get(*venue_index).m_price != top) {
      m_top_prices.set(*venue_index, TopMpidPrice(venue, top));
    } else {
      return;
    }
  }
  update_top_price_model(venue);
}

void TopMpidPriceListModel::add_quote(const BookQuote& quote) {
  if(!quote.m_is_primary_mpid) {
    return;
  }
  ++m_prices[quote.m_venue][quote.m_quote.m_price];
  update_top_price(quote.m_venue, quote.m_quote.m_side);
}

void TopMpidPriceListModel::remove_quote(const BookQuote& quote) {
  if(!quote.m_is_primary_mpid) {
    return;
  }
  auto prices = m_prices.find(quote.m_venue);
  if(prices == m_prices.end()) {
    return;
  }
  auto price = prices->second.find(quote.m_quote.m_price);
  if(price == prices->second.end()) {
    return;
  }
  --price->second;
  if(price->second == 0) {
    prices->second.erase(price);
    if(prices->second.empty()) {
      m_prices.erase(prices);
    }
  }
  update_top_price(quote.m_venue, quote.m_quote.m_side);
}

void TopMpidPriceListModel::on_operation(
    const BookQuoteListModel::Operation& operation) {
  visit(operation,
    [&] (const BookQuoteListModel::AddOperation& operation) {
      add_quote(m_quotes->get(operation.m_index));
    },
    [&] (const BookQuoteListModel::PreRemoveOperation& operation) {
      m_removed_quote = m_quotes->get(operation.m_index);
    },
    [&] (const BookQuoteListModel::RemoveOperation& operation) {
      remove_quote(m_removed_quote);
    },
    [&] (const BookQuoteListModel::UpdateOperation& operation) {
      if(is_top_price_equivalent(
          operation.get_previous(), operation.get_value())) {
        return;
      }
      remove_quote(operation.get_previous());
      add_quote(operation.get_value());
    });
}
