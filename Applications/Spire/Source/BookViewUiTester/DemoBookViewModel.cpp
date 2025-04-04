#include "Spire/BookViewUiTester/DemoBookViewModel.hpp"
#include "Spire/Spire/ListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  auto get_quotes(const BookViewModel& model, Side side) {
    if(side == Side::BID) {
      return model.get_bids();
    }
    return model.get_asks();
  }

  int find_book_quote(const ListModel<BookQuote>& quotes,
      const optional<OrderFields>& preview_order,
      const std::string& mpid, const Money& price) {
    auto i = std::find_if(quotes.begin(), quotes.end(),
      [&] (const BookQuote& quote) {
        return quote.m_mpid == mpid && quote.m_quote.m_price == price;
      });
    if(i == quotes.end()) {
      return -1;
    }
    return std::distance(quotes.begin(), i);
  }
}

DemoBookViewModel::DemoBookViewModel(std::shared_ptr<BookViewModel> model)
  : m_model(std::move(model)),
    m_bbo(BboQuote{Quote(Money(0), 0, Side::BID),
      Quote{Money(std::numeric_limits<Quantity>::max()), 0, Side::ASK},
      second_clock::universal_time()}),
BEAM_SUPPRESS_THIS_INITIALIZER()
    m_bid_operation_connection(m_model->get_bids()->connect_operation_signal(
      std::bind_front(&DemoBookViewModel::on_bid_operation, this))),
    m_ask_operation_connection(m_model->get_asks()->connect_operation_signal(
      std::bind_front(&DemoBookViewModel::on_ask_operation, this))) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

const std::shared_ptr<BookViewModel>& DemoBookViewModel::get_model() const {
  return m_model;
}

void DemoBookViewModel::submit_book_quote(const BookQuote& quote) {
  auto quotes = get_quotes(*m_model, quote.m_quote.m_side);
  auto i = find_book_quote(*quotes, m_model->get_preview_order()->get(),
    quote.m_mpid, quote.m_quote.m_price);
  if(i >= 0) {
    if(quote.m_quote.m_size == 0) {
      quotes->remove(i);
    } else {
      quotes->set(i, quote);
    }
  } else if(quote.m_quote.m_size != 0) {
    quotes->push(quote);
  }
}

void DemoBookViewModel::on_bid_operation(
    const ListModel<BookQuote>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<BookQuote>::AddOperation& operation) {
      auto& quote = m_model->get_bids()->get(operation.m_index);
      if(m_bbo.m_bid.m_price < quote.m_quote.m_price) {
        m_bbo.m_bid = quote.m_quote;
        m_model->get_bbo_quote()->set(m_bbo);
      }
    },
    [&] (const ListModel<BookQuote>::PreRemoveOperation& operation) {
      auto& bids = m_model->get_bids();
      auto& quote = bids->get(operation.m_index);
      if(m_bbo.m_bid.m_price == quote.m_quote.m_price) {
        auto max_price = Money(0);
        auto max_price_index = -1;
        for(auto i = 0; i < bids->get_size(); ++i) {
          auto& price = bids->get(i).m_quote.m_price;
          if(price < m_bbo.m_bid.m_price && price > max_price) {
            max_price = price;
            max_price_index = i;
          }
        }
        if(max_price_index >= 0) {
          m_bbo.m_bid = bids->get(max_price_index).m_quote;
        } else {
          m_bbo.m_bid = Quote{max_price, 0, Side::BID};
        }
        m_model->get_bbo_quote()->set(m_bbo);
      }
    });
}

void DemoBookViewModel::on_ask_operation(
    const ListModel<BookQuote>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<BookQuote>::AddOperation& operation) {
      auto& quote = m_model->get_asks()->get(operation.m_index);
      if(m_bbo.m_ask.m_price > quote.m_quote.m_price) {
        m_bbo.m_ask = quote.m_quote;
        m_model->get_bbo_quote()->set(m_bbo);
      }
    },
    [&] (const ListModel<BookQuote>::PreRemoveOperation& operation) {
      auto& asks = m_model->get_asks();
      auto& quote = asks->get(operation.m_index);
      if(m_bbo.m_ask.m_price == quote.m_quote.m_price) {
        auto min_price = Money(std::numeric_limits<Quantity>::max());
        auto min_price_index = -1;
        for(auto i = 0; i < asks->get_size(); ++i) {
          auto& price = asks->get(i).m_quote.m_price;
          if(price > m_bbo.m_ask.m_price && price < min_price) {
            min_price = price;
            min_price_index = i;
          }
        }
        if(min_price_index >= 0) {
          m_bbo.m_ask = asks->get(min_price_index).m_quote;
        } else {
          m_bbo.m_ask = Quote{min_price, 0, Side::ASK};
        }
        m_model->get_bbo_quote()->set(m_bbo);
      }
    });
}
