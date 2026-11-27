#include "Spire/Ui/DefaultTableAggregator.hpp"
#include "Spire/Ui/AccountListItem.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DistinctTableAggregator.hpp"
#include "Spire/Ui/SumTableAggregator.hpp"

using namespace Beam;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  template<typename... T>
  void visit_as(const AnyRef& value, const auto& visit) {
    auto match = [&]<typename N> {
      if(auto item = any_cast<N>(&value)) {
        visit(*item);
        return true;
      }
      return false;
    };
    (match.template operator()<T>() || ...);
  }
}

void DefaultTableAggregator::add(const AnyRef& value) {
  if(!m_aggregator && value.has_value()) {
    select_aggregator(value);
  }
  if(m_aggregator) {
    m_aggregator->add(value);
  }
}

void DefaultTableAggregator::remove(const AnyRef& value) {
  if(m_aggregator) {
    m_aggregator->remove(value);
  }
}

void DefaultTableAggregator::update(
    const AnyRef& previous, const AnyRef& current) {
  if(m_aggregator) {
    m_aggregator->update(previous, current);
  } else {
    add(current);
  }
}

std::any DefaultTableAggregator::get() const {
  if(m_aggregator) {
    return m_aggregator->get();
  }
  return {};
}

void DefaultTableAggregator::select_aggregator(const AnyRef& value) {
  auto select = [&]<typename T> (const T&) {
    using Aggregator = std::conditional_t<
      (std::is_arithmetic_v<T> && !std::same_as<T, bool>) ||
        std::same_as<T, Quantity> || std::same_as<T, Money>,
      SumTableAggregator<T>, DistinctTableAggregator<T>>;
    m_aggregator = std::make_unique<Aggregator>();
  };
  visit_as<int, unsigned int, long, unsigned long, long long,
    unsigned long long, short, unsigned short, signed char, unsigned char,
    double, float, long double, Quantity, Money, QString, std::string,
    Side, PositionSide, TradeSide, CurrencyId, Ticker, Venue, CountryCode,
    OrderStatus, OrderType, TimeInForce, TimeAndSale::Condition, Scope,
    DirectoryEntry, AccountListItem::Account, Task::State, PositionSideToken,
    date, ptime, time_duration, QColor, QKeySequence, bool>(value, select);
}
