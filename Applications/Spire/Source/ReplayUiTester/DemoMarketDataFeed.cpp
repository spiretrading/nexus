#include "Spire/ReplayUiTester/DemoMarketDataFeed.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto BBO_INTERVAL = 5;
  const auto TIMER_INTERVAL = 100;
  const auto BURST_ODDS = 50;
  const auto MIN_BURST_SIZE = 10;
  const auto MAX_BURST_SIZE = 50;
  const auto BURST_SEED = quint32(0x2545F491);
  const auto LARGE_BURST_PERIOD = minutes(2);
  const auto LARGE_BURST_SIZE = 2000;
  const auto GAP_ACTIVE = minutes(2);
  const auto GAP_QUIET = minutes(5);
  const auto LATE_START_DELAY = hours(1);
  const auto MARKETS = std::vector<std::string>{"XNYS", "TSE", "CHD", "CHI"};
  const auto MPIDS = std::vector<std::string>{"NBFI", "CIBC", "PFSS", "RBCD"};

  class SplitMixGenerator {
    public:
      explicit SplitMixGenerator(quint32 seed)
        : m_state(seed) {}

      int bounded(int highest) {
        return static_cast<int>(
          ((next() >> 32) * static_cast<quint64>(highest)) >> 32);
      }

      int bounded(int lowest, int highest) {
        return lowest + bounded(highest - lowest);
      }

    private:
      quint64 m_state;

      quint64 next() {
        m_state += 0x9E3779B97F4A7C15ULL;
        auto value = m_state;
        value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ULL;
        value = (value ^ (value >> 27)) * 0x94D049BB133111EBULL;
        return value ^ (value >> 31);
      }
  };

  auto to_money(int cents) {
    return cents * Money::CENT;
  }

  QString to_duration_text(time_duration duration) {
    auto total = duration.total_seconds();
    if(total % 3600 == 0) {
      if(total == 3600) {
        return QObject::tr("1 hour");
      }
      return QObject::tr("%1 hours").arg(total / 3600);
    } else if(total % 60 == 0) {
      if(total == 60) {
        return QObject::tr("1 minute");
      }
      return QObject::tr("%1 minutes").arg(total / 60);
    }
    return QObject::tr("%1 seconds").arg(total);
  }

  bool has_bbo_quote(int index) {
    return index % BBO_INTERVAL == 0;
  }

  TimeAndSale make_time_and_sale(
      quint32 seed, int index, int price, ptime timestamp) {
    auto random = SplitMixGenerator(seed + index);
    return TimeAndSale(timestamp, to_money(price + random.bounded(-50, 51)),
      100 * random.bounded(1, 20),
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
      MARKETS[random.bounded(static_cast<int>(MARKETS.size()))],
      MPIDS[random.bounded(static_cast<int>(MPIDS.size()))],
      MPIDS[random.bounded(static_cast<int>(MPIDS.size()))]);
  }

  BboQuote make_bbo_quote(quint32 seed, int index, int price, ptime timestamp) {
    auto random = SplitMixGenerator(seed - index - 1);
    auto bid = price + random.bounded(-40, 21);
    auto ask = bid + random.bounded(1, 30);
    return BboQuote(make_bid(to_money(bid), 100 * random.bounded(1, 20)),
      make_ask(to_money(ask), 100 * random.bounded(1, 20)),
      timestamp - milliseconds(1));
  }
}

DemoMarketDataFeed::DemoMarketDataFeed(Ref<TestEnvironment> environment,
    std::vector<Profile> profiles, ptime start)
    : m_environment(environment.get()),
      m_start(start) {
  auto now = microsec_clock::universal_time();
  for(auto& profile : profiles) {
    if(profile.m_interval <= seconds(0)) {
      continue;
    }
    auto seed = static_cast<quint32>(qHash(to_text(profile.m_ticker)));
    auto timestamp = [&] {
      if(profile.m_pattern == Pattern::LATE_START) {
        return m_start + LATE_START_DELAY;
      }
      return m_start;
    }();
    m_generators.push_back(Generator(std::move(profile), seed,
      100 * (10 + static_cast<int>(seed % 90)), 0, 0, timestamp));
    store(m_generators.back(), now);
  }
  QObject::connect(&m_timer, &QTimer::timeout,
    std::bind_front(&DemoMarketDataFeed::on_timeout, this));
  m_timer.start(TIMER_INTERVAL);
}

QString DemoMarketDataFeed::describe(const Profile& profile) {
  if(profile.m_interval <= seconds(0)) {
    return QObject::tr("No trades.");
  }
  auto rate = [&] {
    auto interval = profile.m_interval.total_microseconds();
    if(interval < 1000000) {
      return QObject::tr("%1 trades per second").arg(1000000.0 / interval);
    } else if(interval == 1000000) {
      return QObject::tr("1 trade per second");
    }
    return QObject::tr("1 trade every %1 seconds").arg(interval / 1000000.0);
  }();
  if(profile.m_pattern == Pattern::BURST) {
    return QObject::tr("%1, with a burst of %2 to %3 trades at the exact same "
      "time about every %4.").arg(rate).arg(MIN_BURST_SIZE).
        arg(MAX_BURST_SIZE).
        arg(to_duration_text(profile.m_interval * BURST_ODDS));
  } else if(profile.m_pattern == Pattern::LARGE_BURST) {
    return QObject::tr("%1, with a burst of %2 trades at the exact same time "
      "every %3.").arg(rate).arg(LARGE_BURST_SIZE).
        arg(to_duration_text(LARGE_BURST_PERIOD));
  } else if(profile.m_pattern == Pattern::GAP) {
    return QObject::tr("%1 for %2, then no trades for %3, repeating.").
      arg(rate).arg(to_duration_text(GAP_ACTIVE)).
      arg(to_duration_text(GAP_QUIET));
  } else if(profile.m_pattern == Pattern::LATE_START) {
    return QObject::tr("No trades for %1 after the start, then %2.").
      arg(to_duration_text(LATE_START_DELAY)).arg(rate);
  }
  return QObject::tr("%1.").arg(rate);
}

std::vector<DemoMarketDataFeed::Burst>
    DemoMarketDataFeed::get_next_large_bursts() const {
  auto bursts = std::vector<Burst>();
  for(auto& generator : m_generators) {
    if(generator.m_profile.m_pattern == Pattern::LARGE_BURST) {
      bursts.push_back(find_next_burst(generator));
    }
  }
  return bursts;
}

connection DemoMarketDataFeed::connect_large_burst_signal(
    const LargeBurstSignal::slot_type& slot) const {
  return m_large_burst_signal.connect(slot);
}

void DemoMarketDataFeed::advance(Generator& generator) const {
  ++generator.m_next;
  if(generator.m_burst != 0) {
    --generator.m_burst;
    return;
  }
  auto& profile = generator.m_profile;
  auto previous = generator.m_timestamp;
  generator.m_timestamp += profile.m_interval;
  if(profile.m_pattern == Pattern::BURST) {
    auto random =
      SplitMixGenerator((generator.m_seed ^ BURST_SEED) + generator.m_next);
    if(random.bounded(BURST_ODDS) == 0) {
      generator.m_burst =
        random.bounded(MIN_BURST_SIZE, MAX_BURST_SIZE + 1) - 1;
    }
  } else if(profile.m_pattern == Pattern::LARGE_BURST) {
    auto period = LARGE_BURST_PERIOD.total_milliseconds();
    if((previous - m_start).total_milliseconds() / period !=
        (generator.m_timestamp - m_start).total_milliseconds() / period) {
      generator.m_burst = LARGE_BURST_SIZE - 1;
    }
  } else if(profile.m_pattern == Pattern::GAP) {
    auto cycle = (GAP_ACTIVE + GAP_QUIET).total_milliseconds();
    auto offset =
      (generator.m_timestamp - m_start).total_milliseconds() % cycle;
    if(offset >= GAP_ACTIVE.total_milliseconds()) {
      generator.m_timestamp += milliseconds(cycle - offset);
    }
  }
}

DemoMarketDataFeed::Burst DemoMarketDataFeed::find_next_burst(
    const Generator& generator) const {
  auto next = generator;
  while(true) {
    auto is_idle = next.m_burst == 0;
    advance(next);
    if(is_idle && next.m_burst != 0) {
      return Burst(next.m_profile.m_ticker, next.m_timestamp, next.m_burst + 1);
    }
  }
}

void DemoMarketDataFeed::store(Generator& generator, ptime end) {
  auto time_and_sales = std::vector<SequencedTickerTimeAndSale>();
  auto bbo_quotes = std::vector<SequencedTickerBboQuote>();
  auto& ticker = generator.m_profile.m_ticker;
  while(generator.m_timestamp <= end) {
    if(has_bbo_quote(generator.m_next)) {
      bbo_quotes.push_back(SequencedValue(IndexedValue(
        make_bbo_quote(generator.m_seed, generator.m_next, generator.m_price,
          generator.m_timestamp), ticker),
        Beam::Sequence(bbo_quotes.size() + 1)));
    }
    time_and_sales.push_back(SequencedValue(IndexedValue(
      make_time_and_sale(generator.m_seed, generator.m_next, generator.m_price,
        generator.m_timestamp), ticker),
      Beam::Sequence(time_and_sales.size() + 1)));
    advance(generator);
  }
  auto& data_store =
    m_environment->get_market_data_environment().get_data_store();
  data_store.store(bbo_quotes);
  data_store.store(time_and_sales);
}

void DemoMarketDataFeed::on_timeout() {
  auto now = microsec_clock::universal_time();
  for(auto& generator : m_generators) {
    auto& ticker = generator.m_profile.m_ticker;
    auto burst = optional<Burst>();
    while(generator.m_timestamp <= now) {
      if(generator.m_profile.m_pattern == Pattern::LARGE_BURST &&
          generator.m_burst == LARGE_BURST_SIZE - 1) {
        burst = Burst(ticker, generator.m_timestamp, LARGE_BURST_SIZE);
      }
      if(has_bbo_quote(generator.m_next)) {
        m_environment->publish(ticker, make_bbo_quote(generator.m_seed,
          generator.m_next, generator.m_price, generator.m_timestamp));
      }
      m_environment->publish(ticker, make_time_and_sale(generator.m_seed,
        generator.m_next, generator.m_price, generator.m_timestamp));
      advance(generator);
    }
    if(burst) {
      m_large_burst_signal(*burst, find_next_burst(generator));
    }
  }
}
