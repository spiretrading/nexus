#include "Spire/BookViewUiTester/DemoBookViewModel.hpp"
#include <QRandomGenerator>
#include "Spire/Spire/ListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  std::time_t to_time_t_milliseconds(ptime pt) {
    return (pt - ptime(gregorian::date(1970, 1, 1))).total_milliseconds();
  }

  Side get_side(CancelKeyBindingsModel::Operation operation) {
    if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK ||
        operation == CancelKeyBindingsModel::Operation::OLDEST_ASK ||
        operation == CancelKeyBindingsModel::Operation::ALL_ASKS ||
        operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK ||
        operation == CancelKeyBindingsModel::Operation::FURTHEST_ASK) {
      return Side::ASK;
    } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID ||
        operation == CancelKeyBindingsModel::Operation::OLDEST_BID ||
        operation == CancelKeyBindingsModel::Operation::ALL_BIDS ||
        operation == CancelKeyBindingsModel::Operation::CLOSEST_BID ||
        operation == CancelKeyBindingsModel::Operation::FURTHEST_BID) {
      return Side::BID;
    }
    return Side::NONE;
  }

  auto get_quotes(const BookViewModel& model, Side side) {
    if(side == Side::BID) {
      return model.get_bids();
    }
    return model.get_asks();
  }

  auto get_orders(const BookViewModel& model, Side side) {
    if(side == Side::BID) {
      return model.get_bid_orders();
    }
    return model.get_ask_orders();
  }

  int find_book_quote(const ListModel<BookQuote>& quotes,
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

  int find_order(const ListModel<BookViewModel::UserOrder>& orders,
      const Destination& destination, const Money& price) {
    auto i = std::find_if(orders.begin(), orders.end(),
      [&] (const BookViewModel::UserOrder& order) {
        return order.m_destination == destination && order.m_price == price;
      });
    if(i == orders.end()) {
      return -1;
    }
    return std::distance(orders.begin(), i);
  }

  int find_order(const ListModel<BookViewModel::UserOrder>& orders,
      const Destination& destination, const Money& price,
      const Quantity& size) {
    auto i = std::find_if(orders.begin(), orders.end(),
      [&] (const BookViewModel::UserOrder& order) {
        return order.m_destination == destination && order.m_price == price &&
          order.m_size == size;
      });
    if(i == orders.end()) {
      return -1;
    }
    return std::distance(orders.begin(), i);
  }

  auto make_user_order(const DemoBookViewModel::OrderInfo& order) {
    return BookViewModel::UserOrder(order.m_order_fields.m_destination,
      order.m_order_fields.m_price, order.m_order_fields.m_quantity,
      order.m_status);
  }

  void update_order(const OrderFields& order_fields, OrderStatus status,
      ListModel<BookQuote>& quotes,
      ListModel<BookViewModel::UserOrder>& orders) {
    auto order_index = find_order(orders, order_fields.m_destination,
      order_fields.m_price, order_fields.m_quantity);
    if(order_index < 0) {
      return;
    }
    auto mpid = "@" + order_fields.m_destination;
    auto quote_index = find_book_quote(quotes, mpid, order_fields.m_price);
    if(quote_index < 0) {
      return;
    }
    auto remaining_size = quotes.get(quote_index).m_quote.m_size;
    remaining_size -= order_fields.m_quantity;
    remaining_size = std::max(Quantity(0), remaining_size);
    auto book_quote = quotes.get(quote_index);
    book_quote.m_quote.m_size = remaining_size;
    quotes.set(quote_index, book_quote);
    auto order = orders.get(order_index);
    order.m_status = status;
    orders.set(order_index, order);
    order_index = find_order(orders, order_fields.m_destination,
      order_fields.m_price, order_fields.m_quantity);
    if(order_index < 0) {
      return;
    }
    order.m_status = OrderStatus::NONE;
    orders.set(order_index, order);
    orders.remove(order_index);
  }

  void execute_cancel(const OrderFields& order_to_cancel,
      ListModel<BookQuote>& quotes, ListModel<BookViewModel::UserOrder>& orders) {
    auto random_generator =
      QRandomGenerator(to_time_t_milliseconds(microsec_clock::universal_time()));
    QTimer::singleShot(random_generator.bounded(5000), [=, &quotes, &orders] {
      update_order(order_to_cancel, OrderStatus::CANCELED, quotes, orders);
    });
  }

  template<typename T>
  auto to_base(const T& i) {
    return i;
  }

  template<typename T>
  auto to_base(const std::reverse_iterator<T>& i) {
    return std::next(i).base();
  }
}

DemoBookViewModel::DemoBookViewModel(std::shared_ptr<BookViewModel> model)
  : m_model(std::move(model)),
    m_bbo(BboQuote{Quote(Money(0), 0, Side::BID),
      Quote{Money(std::numeric_limits<Quantity>::max()), 0, Side::ASK},
      second_clock::universal_time()}),
    m_bid_operation_connection(m_model->get_bids()->connect_operation_signal(
      std::bind_front(&DemoBookViewModel::on_bid_operation, this))),
    m_ask_operation_connection(m_model->get_asks()->connect_operation_signal(
      std::bind_front(&DemoBookViewModel::on_ask_operation, this))) {}

const std::shared_ptr<BookViewModel>& DemoBookViewModel::get_model() const {
  return m_model;
}

void DemoBookViewModel::update_order_status(const OrderInfo& order) {
  auto quotes = get_quotes(*m_model, order.m_order_fields.m_side);
  auto orders = get_orders(*m_model, order.m_order_fields.m_side);
  update_order(order.m_order_fields, order.m_status, *quotes, *orders);
  if(order.m_status == OrderStatus::FILLED ||
      order.m_status == OrderStatus::CANCELED ||
      order.m_status == OrderStatus::REJECTED) {
    for(auto i = m_orders.rbegin(); i != m_orders.rend(); ++i) {
      if(i->m_order_fields.m_destination == order.m_order_fields.m_destination &&
          i->m_order_fields.m_price == order.m_order_fields.m_price) {
        m_orders.erase(std::next(i).base());
        break;
      }
    }
  }
}

void DemoBookViewModel::submit_book_quote(const BookQuote& quote) {
  auto quotes = get_quotes(*m_model, quote.m_quote.m_side);
  auto i = find_book_quote(*quotes, quote.m_mpid, quote.m_quote.m_price);
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

void DemoBookViewModel::submit_order(const OrderInfo& order) {
  auto quotes = get_quotes(*m_model, order.m_order_fields.m_side);
  auto orders = get_orders(*m_model, order.m_order_fields.m_side);
  auto order_index = find_order(*orders,
    order.m_order_fields.m_destination, order.m_order_fields.m_price);
  if(order_index >= 0) {
    auto quote_index = find_book_quote(*quotes,
      "@" + order.m_order_fields.m_destination, order.m_order_fields.m_price);
    if(quote_index < 0) {
      return;
    }
    auto book_quote = quotes->get(quote_index);
    auto quantity = book_quote.m_quote.m_size;
    if(order.m_status == OrderStatus::NEW) {
      book_quote.m_quote.m_size =
        book_quote.m_quote.m_size + order.m_order_fields.m_quantity;
      quotes->set(quote_index, book_quote);
      orders->push(make_user_order(order));
      m_orders.push_back(order);
    } else {
      update_order(order.m_order_fields, order.m_status, *quotes, *orders);
    }
  } else if(order.m_status == OrderStatus::NEW) {
    auto quote = BookQuote("@" + order.m_order_fields.m_destination, false, "",
      Quote{order.m_order_fields.m_price, order.m_order_fields.m_quantity,
        order.m_order_fields.m_side}, second_clock::local_time());
    quotes->push(std::move(quote));
    orders->push(make_user_order(order));
    m_orders.push_back(order);
  }
}

void DemoBookViewModel::cancel_orders(
    CancelKeyBindingsModel::Operation operation,
    const optional<std::tuple<Destination, Money>>& order_key) {
  if(m_orders.empty()) {
    return;
  }
  auto orders_to_cancel = std::vector<OrderFields>();
  auto expected_side = get_side(operation);
  if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT) {
    orders_to_cancel.push_back(m_orders.back().m_order_fields);
    m_orders.pop_back();
  } else if(operation == CancelKeyBindingsModel::Operation::OLDEST) {
    orders_to_cancel.push_back(m_orders.front().m_order_fields);
    m_orders.erase(m_orders.begin());
  } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK ||
      operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID ||
      operation == CancelKeyBindingsModel::Operation::OLDEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::OLDEST_BID) {
    auto cancel = [&] (auto begin, auto end) {
      for(auto i = begin; i != end; ++i) {
        if(i->m_order_fields.m_side == expected_side) {
          if(order_key) {
            if(get<0>(*order_key) == i->m_order_fields.m_destination &&
                get<1>(*order_key) == i->m_order_fields.m_price) {
              orders_to_cancel.push_back(i->m_order_fields);
              m_orders.erase(to_base(i));
              break;
            }
          } else {
            orders_to_cancel.push_back(i->m_order_fields);
            m_orders.erase(to_base(i));
            break;
          }
        }
      }
    };
    if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK ||
        operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID) {
      cancel(m_orders.rbegin(), m_orders.rend());
    } else {
      cancel(m_orders.begin(), m_orders.end());
    }
  } else if(operation == CancelKeyBindingsModel::Operation::ALL) {
    for(auto& order : m_orders) {
      orders_to_cancel.push_back(order.m_order_fields);
    }
    m_orders.clear();
  } else if(operation == CancelKeyBindingsModel::Operation::ALL_ASKS ||
      operation == CancelKeyBindingsModel::Operation::ALL_BIDS) {
    std::erase_if(m_orders, [&] (const auto& order) {
      if(order.m_order_fields.m_side != expected_side) {
        return false;
      }
      if(order_key) {
        if(get<0>(*order_key) == order.m_order_fields.m_destination &&
            get<1>(*order_key) == order.m_order_fields.m_price) {
          orders_to_cancel.push_back(order.m_order_fields);
          return true;
        }
      } else {
        orders_to_cancel.push_back(order.m_order_fields);
        return true;
      }
      return false;
    });
  } else if(operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::CLOSEST_BID ||
      operation == CancelKeyBindingsModel::Operation::FURTHEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::FURTHEST_BID) {
    auto cancel = [&] (auto begin, auto end, auto flip) {
      auto closest_iterator = end;
      auto closest_price = boost::optional<Money>();
      for(auto i = begin; i != end; ++i) {
        if(i->m_order_fields.m_side == expected_side) {
          auto price = i->m_order_fields.m_price;
          if(price != Money::ZERO) {
            if(!closest_price) {
              closest_price = price;
              closest_iterator = i;
            } else if(closest_price && (price < *closest_price) == flip) {
              closest_price = price;
              closest_iterator = i;
            }
          }
        }
      }
      if(closest_iterator != end) {
        orders_to_cancel.push_back(closest_iterator->m_order_fields);
        m_orders.erase(to_base(closest_iterator));
      }
    };
    if(operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK ||
        operation == CancelKeyBindingsModel::Operation::FURTHEST_BID) {
      cancel(m_orders.begin(), m_orders.end(), true);
    } else {
      cancel(m_orders.begin(), m_orders.end(), false);
    }
  }
  if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT ||
      operation == CancelKeyBindingsModel::Operation::ALL ||
      operation == CancelKeyBindingsModel::Operation::OLDEST) {
    for(auto& order_to_cancel : orders_to_cancel) {
      if(order_to_cancel.m_side == Side::BID) {
        execute_cancel(order_to_cancel, *m_model->get_bids(),
          *m_model->get_bid_orders());
      } else {
        execute_cancel(order_to_cancel, *m_model->get_asks(),
          *m_model->get_ask_orders());
      }
    }
  } else {
    auto [side, orders, quotes] = [&] {
      if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID ||
        operation == CancelKeyBindingsModel::Operation::OLDEST_BID ||
        operation == CancelKeyBindingsModel::Operation::CLOSEST_BID ||
        operation == CancelKeyBindingsModel::Operation::FURTHEST_BID ||
        operation == CancelKeyBindingsModel::Operation::ALL_BIDS) {
        return std::tuple(Side::BID, m_model->get_bid_orders(),
          m_model->get_bids());
      }
      return std::tuple(Side::ASK, m_model->get_ask_orders(),
        m_model->get_asks());
    }();
    for(auto& order_to_cancel : orders_to_cancel) {
      execute_cancel(order_to_cancel, *quotes, *orders);
    }
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
