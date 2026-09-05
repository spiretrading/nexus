#include "Spire/BookView/TopMpidPriceListModel.hpp"
#include <boost/optional/optional.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  optional<Money> find_top_price(
      const BookQuoteListModel& quotes, Venue venue) {
    auto top = optional<Money>();
    for(auto i = 0; i != quotes.get_size(); ++i) {
      auto& quote = quotes.get(i);
      if(quote.m_is_primary_mpid && quote.m_venue == venue) {
        auto direction = get_direction(quote.m_quote.m_side);
        if(!top || direction * quote.m_quote.m_price > direction * *top) {
          top = quote.m_quote.m_price;
        }
      }
    }
    return top;
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

std::shared_ptr<TopMpidPriceListModel::TopPriceModel>
    TopMpidPriceListModel::get_top_price(Venue venue) {
  auto& model = m_top_price_models[venue];
  if(!model) {
    model =
      std::make_shared<LocalValueModel<optional<Money>>>(find_price(venue));
  }
  return model;
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

void TopMpidPriceListModel::add_quote(const BookQuote& quote) {
  if(!quote.m_is_primary_mpid) {
    return;
  }
  auto venue_index = find_index(quote.m_venue);
  if(!venue_index) {
    m_indexes[quote.m_venue] = m_top_prices.get_size();
    m_top_prices.push(TopMpidPrice(quote.m_venue, quote.m_quote.m_price));
    update_top_price_model(quote.m_venue);
    return;
  }
  auto& top_mpid = m_top_prices.get(*venue_index);
  auto direction = get_direction(quote.m_quote.m_side);
  if(direction * quote.m_quote.m_price > direction * top_mpid.m_price) {
    m_top_prices.set(
      *venue_index, TopMpidPrice(quote.m_venue, quote.m_quote.m_price));
    update_top_price_model(quote.m_venue);
  }
}

void TopMpidPriceListModel::remove_quote(const BookQuote& quote) {
  if(!quote.m_is_primary_mpid) {
    return;
  }
  auto venue_index = find_index(quote.m_venue);
  if(!venue_index) {
    return;
  }
  auto venue = m_top_prices.get(*venue_index).m_venue;
  if(m_top_prices.get(*venue_index).m_price != quote.m_quote.m_price) {
    return;
  }
  if(auto top = find_top_price(*m_quotes, venue)) {
    m_top_prices.set(*venue_index, TopMpidPrice(venue, *top));
  } else {
    m_indexes.erase(venue);
    for(auto& entry : m_indexes) {
      if(entry.second > *venue_index) {
        --entry.second;
      }
    }
    m_top_prices.remove(*venue_index);
  }
  update_top_price_model(venue);
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
      remove_quote(operation.get_previous());
      add_quote(operation.get_value());
    });
}
