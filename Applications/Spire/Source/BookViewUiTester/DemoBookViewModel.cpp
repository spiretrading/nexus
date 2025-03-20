#include "Spire/BookViewUiTester/DemoBookViewModel.hpp"
#include <ranges>
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

  auto make_user_order(const DemoBookViewModel::OrderInfo& order) {
    return BookViewModel::UserOrder(order.m_order_fields.m_destination,
      order.m_order_fields.m_price, order.m_order_fields.m_quantity,
      order.m_status);
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

void DemoBookViewModel::submit_order(const OrderInfo& order_info) {
  if(order_info.m_status == OrderStatus::NEW) {
    m_orders.push_back(order_info);
    update_order(order_info);
  } else {
    auto i = std::find_if(m_orders.begin(), m_orders.end(), [&] (auto& order) {
      return order.m_order_fields.m_destination ==
        order_info.m_order_fields.m_destination &&
        order.m_order_fields.m_price == order_info.m_order_fields.m_price &&
        order.m_order_fields.m_quantity == order_info.m_order_fields.m_quantity;
    });
    if(i != m_orders.end()) {
      update_order(order_info);
      m_orders.erase(i);
    } else {
      auto orders = m_orders | std::views::filter([&] (const auto& order) {
        return order.m_order_fields.m_destination ==
          order_info.m_order_fields.m_destination &&
          order.m_order_fields.m_price == order_info.m_order_fields.m_price &&
          order.m_order_fields.m_quantity >
            order_info.m_order_fields.m_quantity;
      });
      if(!orders.empty()) {
        update_order(order_info);
        orders.front().m_order_fields.m_quantity -=
          order_info.m_order_fields.m_quantity;
      }
    }
  }
}

void DemoBookViewModel::cancel_orders(
    CancelKeyBindingsModel::Operation operation,
    const optional<BookViewWindow::CancelCriteria>& criteria) {
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
          if(criteria) {
            if(criteria->m_destination == i->m_order_fields.m_destination &&
                criteria->m_price == i->m_order_fields.m_price) {
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
      if(criteria) {
        if(criteria->m_destination == order.m_order_fields.m_destination &&
            criteria->m_price == order.m_order_fields.m_price) {
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
  for(auto& order_to_cancel : orders_to_cancel) {
    execute_cancel(order_to_cancel);
  }
}

void DemoBookViewModel::execute_cancel(const OrderFields& order_to_cancel) {
  auto random_generator =
    QRandomGenerator(to_time_t_milliseconds(microsec_clock::universal_time()));
  QTimer::singleShot(random_generator.bounded(5000), [=] {
    update_order({order_to_cancel, OrderStatus::CANCELED});
  });
}

void DemoBookViewModel::update_order(const OrderInfo& order_info) {
  auto orders = get_orders(*m_model, order_info.m_order_fields.m_side);
  auto order_index = find_order(*orders,
    order_info.m_order_fields.m_destination, order_info.m_order_fields.m_price);
  if(order_index >= 0) {
    if(order_info.m_status == OrderStatus::NEW) {
      auto order = orders->get(order_index);
      order.m_size += order_info.m_order_fields.m_quantity;
      order.m_status = order_info.m_status;
      orders->set(order_index, order);
    } else {
      auto order = orders->get(order_index);
      order.m_size -= order_info.m_order_fields.m_quantity;
      order.m_status = order_info.m_status;
      orders->set(order_index, order);
      if(order.m_size <= 0) {
        orders->remove(order_index);
      }
    }
  } else if(order_info.m_status == OrderStatus::NEW) {
    orders->push(make_user_order(order_info));
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
