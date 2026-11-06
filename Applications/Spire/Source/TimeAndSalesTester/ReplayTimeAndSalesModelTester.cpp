#include <atomic>
#include <deque>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include <QApplication>
#include <QTimer>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"
#include "Spire/Playback/ReplaySession.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/TimeAndSales/ReplayTimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto BASE = time_from_string("2024-06-01 10:00:00");
  const auto NOW = time_from_string("2024-06-01 11:00:00");
  const auto NOW_OFFSET = NOW - BASE;
  const auto TICKER = parse_ticker("ABX.TSX");
  const auto WAIT_LIMIT = seconds(5);

  TimeAndSale make_time_and_sale(time_duration offset) {
    return TimeAndSale(BASE + offset, Money::ONE, 100,
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
      "XNYS");
  }

  class TimeAndSalesQueryCountingClient {
    public:
      TimeAndSalesQueryCountingClient(
          MarketDataClient client, std::shared_ptr<std::atomic_int> count)
        : m_client(std::move(client)),
          m_count(std::move(count)) {}

      template<typename Q, typename T>
      void query(const Q& query, ScopedQueueWriter<T> queue) {
        if constexpr(std::is_same_v<T, SequencedTimeAndSale>) {
          ++*m_count;
        }
        m_client.query(query, std::move(queue));
      }

      std::vector<TickerInfo> query(const TickerInfoQuery& query) {
        return m_client.query(query);
      }

      TickerSnapshot load_snapshot(const Ticker& ticker) {
        return m_client.load_snapshot(ticker);
      }

      SequencedSessionTechnicals load_session_technicals(
          const Ticker& ticker) {
        return m_client.load_session_technicals(ticker);
      }

      std::vector<TickerInfo> load_ticker_info_from_prefix(
          const std::string& prefix) {
        return m_client.load_ticker_info_from_prefix(prefix);
      }

      void close() {
        m_client.close();
      }

    private:
      MarketDataClient m_client;
      std::shared_ptr<std::atomic_int> m_count;
  };

  struct Fixture {
    TestEnvironment m_environment;
    TestClients m_clients;
    FixedTimeClient m_time_client;
    std::shared_ptr<ReplaySession> m_session;
    int m_sequence;
    int m_bbo_sequence;

    Fixture(time_duration position, time_duration start = seconds(0))
      : m_environment(NOW),
        m_clients(Ref(m_environment)),
        m_time_client(NOW),
        m_session(std::make_shared<ReplaySession>(
          Timeline(BASE + start, hours(2)), position - start, Decimal(1),
          TimeClient(&m_time_client))),
        m_sequence(0),
        m_bbo_sequence(0) {}

    void store_bbo(time_duration offset, Money bid, Money ask) {
      m_environment.get_market_data_environment().get_data_store().store(
        SequencedValue(IndexedValue(BboQuote(make_bid(bid, 100),
          make_ask(ask, 100), BASE + offset), TICKER),
          Beam::Sequence(++m_bbo_sequence)));
    }

    void store(const std::vector<time_duration>& offsets) {
      auto time_and_sales = std::vector<SequencedTickerTimeAndSale>();
      for(auto& offset : offsets) {
        time_and_sales.push_back(SequencedValue(
          IndexedValue(make_time_and_sale(offset), TICKER),
          Beam::Sequence(++m_sequence)));
      }
      m_environment.get_market_data_environment().get_data_store().store(
        time_and_sales);
    }

    void set_position(time_duration position) {
      m_session->get_playhead()->set(
        BASE + position - m_session->get_timeline()->get().m_start);
    }

    std::shared_ptr<ReplayTimeAndSalesModel> make_model() {
      return std::make_shared<ReplayTimeAndSalesModel>(
        TICKER, m_clients.get_market_data_client(), m_session);
    }

    std::shared_ptr<ReplayTimeAndSalesModel> make_model(
        std::shared_ptr<std::atomic_int> count) {
      return std::make_shared<ReplayTimeAndSalesModel>(TICKER,
        MarketDataClient(std::in_place_type<TimeAndSalesQueryCountingClient>,
          m_clients.get_market_data_client(), std::move(count)),
        m_session);
    }
  };

  std::vector<time_duration> get_offsets(const TimeAndSalesModel& model) {
    auto offsets = std::vector<time_duration>();
    for(auto i = 0; i < model.get_size(); ++i) {
      offsets.push_back(model.get(i).m_time_and_sale->m_timestamp - BASE);
    }
    return offsets;
  }

  void wait_until(auto&& predicate) {
    auto timer = QTimer();
    timer.setSingleShot(true);
    timer.start(static_cast<int>(WAIT_LIMIT.total_milliseconds()));
    while(!predicate()) {
      REQUIRE(timer.isActive());
      QApplication::processEvents(QEventLoop::WaitForMoreEvents);
    }
  }

  void wait_for(time_duration duration) {
    auto loop = QEventLoop();
    QTimer::singleShot(static_cast<int>(duration.total_milliseconds()), &loop,
      &QEventLoop::quit);
    loop.exec();
  }
}

TEST_SUITE("ReplayTimeAndSalesModel") {
  UI_TEST_CASE("load_older_ends_at_the_position") {
    auto fixture = Fixture(duration_from_string("00:00:02.500"));
    fixture.store({seconds(1), seconds(2), seconds(3)});
    auto model = fixture.make_model();
    REQUIRE(model->get_size() == 0);
    wait(model->load_older(10));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2)});
  }

  UI_TEST_CASE("load_older_pages") {
    auto fixture = Fixture(seconds(10));
    fixture.store({seconds(1), seconds(2), seconds(3), seconds(4)});
    auto model = fixture.make_model();
    wait(model->load_older(2));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(3), seconds(4)});
    wait(model->load_older(2));
    REQUIRE(get_offsets(*model) == std::vector<time_duration>{
      seconds(1), seconds(2), seconds(3), seconds(4)});
  }

  UI_TEST_CASE("load_older_stops_at_the_timeline_start") {
    auto fixture = Fixture(seconds(10), duration_from_string("00:00:01.500"));
    fixture.store({seconds(1), seconds(2), seconds(3)});
    auto model = fixture.make_model();
    wait(model->load_older(10));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(2), seconds(3)});
  }

  UI_TEST_CASE("stepping_forward_appends_to_the_back") {
    auto fixture = Fixture(seconds(0));
    fixture.store({seconds(1), seconds(2), seconds(3), seconds(4)});
    auto model = fixture.make_model();
    auto operations = std::deque<TimeAndSalesModel::Operation>();
    auto connection = scoped_connection(model->connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    fixture.set_position(duration_from_string("00:00:01.500"));
    wait_until([&] {
      return model->get_size() == 1;
    });
    REQUIRE(get_offsets(*model) == std::vector<time_duration>{seconds(1)});
    fixture.set_position(seconds(3));
    wait_until([&] {
      return model->get_size() == 3;
    });
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2), seconds(3)});
    auto additions = 0;
    for(auto& operation : operations) {
      REQUIRE(!std::get_if<TimeAndSalesModel::RemoveOperation>(&operation));
      if(std::get_if<TimeAndSalesModel::AddOperation>(&operation)) {
        ++additions;
      }
    }
    REQUIRE(additions == 3);
  }

  UI_TEST_CASE("stepping_back_removes_from_the_back") {
    auto fixture = Fixture(seconds(3));
    fixture.store({seconds(1), seconds(2), seconds(3)});
    auto model = fixture.make_model();
    wait(model->load_older(10));
    REQUIRE(model->get_size() == 3);
    auto operations = std::deque<TimeAndSalesModel::Operation>();
    auto connection = scoped_connection(model->connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    fixture.set_position(duration_from_string("00:00:01.500"));
    REQUIRE(get_offsets(*model) == std::vector<time_duration>{seconds(1)});
    auto removals = 0;
    for(auto& operation : operations) {
      REQUIRE(!std::get_if<TimeAndSalesModel::AddOperation>(&operation));
      if(std::get_if<TimeAndSalesModel::RemoveOperation>(&operation)) {
        ++removals;
      }
    }
    REQUIRE(removals == 2);
    fixture.set_position(seconds(3));
    wait_until([&] {
      return model->get_size() == 3;
    });
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2), seconds(3)});
  }

  UI_TEST_CASE("playing_appends_up_to_the_position") {
    auto fixture = Fixture(seconds(0));
    fixture.store({seconds(1), seconds(2), seconds(3), seconds(4)});
    auto model = fixture.make_model();
    fixture.m_session->play();
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REPLAYING);
    fixture.set_position(seconds(1));
    fixture.set_position(seconds(2));
    wait_until([&] {
      return model->get_size() == 2;
    });
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2)});
    fixture.set_position(seconds(3));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2), seconds(3)});
  }

  UI_TEST_CASE("seeking_back_within_the_list_does_not_query") {
    auto fixture = Fixture(seconds(30));
    auto offsets = std::vector<time_duration>();
    for(auto i = 1; i <= 40; ++i) {
      offsets.push_back(seconds(i));
    }
    fixture.store(offsets);
    auto count = std::make_shared<std::atomic_int>(0);
    auto model = fixture.make_model(count);
    wait(model->load_older(10));
    wait(model->load_older(10));
    REQUIRE(model->get_size() == 20);
    auto queries = count->load();
    fixture.set_position(seconds(25));
    REQUIRE(model->get_size() == 15);
    REQUIRE(get_offsets(*model).back() == seconds(25));
    fixture.set_position(seconds(26));
    REQUIRE(model->get_size() == 16);
    REQUIRE(get_offsets(*model).back() == seconds(26));
    wait_for(milliseconds(200));
    REQUIRE(*count == queries);
  }

  UI_TEST_CASE("seeking_before_the_front_replaces") {
    auto fixture = Fixture(seconds(102));
    fixture.store({seconds(1), seconds(2), seconds(100), seconds(101)});
    auto model = fixture.make_model();
    wait(model->load_older(2));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(100), seconds(101)});
    auto operations = std::deque<TimeAndSalesModel::Operation>();
    auto connection = scoped_connection(model->connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    fixture.set_position(seconds(5));
    REQUIRE(operations.empty());
    wait_until([&] {
      return !operations.empty() &&
        std::get_if<TimeAndSalesModel::EndTransaction>(&operations.back());
    });
    REQUIRE(std::get_if<TimeAndSalesModel::StartTransaction>(
      &operations.front()));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2)});
  }

  UI_TEST_CASE("seeking_far_forward_replaces") {
    auto fixture = Fixture(seconds(5));
    fixture.store({seconds(1), seconds(2), seconds(100), seconds(101),
      seconds(102), seconds(103)});
    auto model = fixture.make_model();
    wait(model->load_older(2));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2)});
    fixture.set_position(seconds(110));
    wait_until([&] {
      return model->get_size() != 0 &&
        model->get(0).m_time_and_sale->m_timestamp == BASE + seconds(102);
    });
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(102), seconds(103)});
  }

  UI_TEST_CASE("seeking_forward_merges_the_page_before_the_position") {
    auto fixture = Fixture(seconds(5));
    fixture.store({seconds(1), seconds(2), seconds(100)});
    auto model = fixture.make_model();
    wait(model->load_older(10));
    REQUIRE(model->get_size() == 2);
    auto operations = std::deque<TimeAndSalesModel::Operation>();
    auto connection = scoped_connection(model->connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    fixture.set_position(seconds(110));
    wait_until([&] {
      return model->get_size() == 3;
    });
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2), seconds(100)});
    for(auto& operation : operations) {
      REQUIRE(!std::get_if<TimeAndSalesModel::RemoveOperation>(&operation));
    }
  }

  UI_TEST_CASE("moving_the_timeline_start_removes_from_the_front") {
    auto fixture = Fixture(seconds(10));
    fixture.store({seconds(1), seconds(2), seconds(3)});
    auto model = fixture.make_model();
    wait(model->load_older(10));
    REQUIRE(model->get_size() == 3);
    fixture.m_session->get_timeline()->set(
      Timeline(BASE + duration_from_string("00:00:01.500"), hours(2)));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(2), seconds(3)});
    wait(model->load_older(10));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(2), seconds(3)});
  }

  UI_TEST_CASE("moving_the_timeline_start_back_replaces") {
    auto fixture = Fixture(seconds(110), seconds(100));
    fixture.store({seconds(1), seconds(2), seconds(105), seconds(106)});
    auto model = fixture.make_model();
    wait(model->load_older(2));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(105), seconds(106)});
    auto operations = std::deque<TimeAndSalesModel::Operation>();
    auto connection = scoped_connection(model->connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    fixture.m_session->get_timeline()->set(Timeline(BASE, hours(2)));
    wait_until([&] {
      return !operations.empty() &&
        std::get_if<TimeAndSalesModel::EndTransaction>(&operations.back());
    });
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2)});
  }

  UI_TEST_CASE(
      "real_time_appends_from_the_back_and_pausing_hides_later_trades") {
    auto fixture = Fixture(NOW_OFFSET);
    fixture.m_environment.publish(TICKER, make_time_and_sale(seconds(1)));
    fixture.m_environment.publish(TICKER, make_time_and_sale(seconds(2)));
    auto model = fixture.make_model();
    wait(model->load_older(1));
    REQUIRE(get_offsets(*model) == std::vector<time_duration>{seconds(2)});
    fixture.m_session->play();
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REAL_TIME);
    fixture.m_environment.publish(
      TICKER, make_time_and_sale(duration_from_string("00:59:59.500")));
    wait_until([&] {
      return model->get_size() == 2;
    });
    fixture.m_environment.publish(
      TICKER, make_time_and_sale(duration_from_string("01:00:00.500")));
    wait_until([&] {
      return model->get_size() == 3;
    });
    fixture.m_session->pause();
    REQUIRE(get_offsets(*model) == std::vector<time_duration>{
      seconds(2), duration_from_string("00:59:59.500")});
    fixture.m_time_client.set(NOW + duration_from_string("00:00:01"));
    fixture.set_position(duration_from_string("01:00:01"));
    REQUIRE(get_offsets(*model) == std::vector<time_duration>{seconds(2),
      duration_from_string("00:59:59.500"),
      duration_from_string("01:00:00.500")});
    wait(model->load_older(10));
    REQUIRE(get_offsets(*model) == std::vector<time_duration>{seconds(1),
      seconds(2), duration_from_string("00:59:59.500"),
      duration_from_string("01:00:00.500")});
  }

  UI_TEST_CASE("reading_near_the_present_waits_for_new_data") {
    auto fixture = Fixture(duration_from_string("00:59:59"));
    fixture.store({duration_from_string("00:59:59.500")});
    auto count = std::make_shared<std::atomic_int>(0);
    auto model = fixture.make_model(count);
    fixture.m_session->play();
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REPLAYING);
    fixture.set_position(duration_from_string("00:59:59.500"));
    wait_until([&] {
      return model->get_size() == 1;
    });
    REQUIRE(*count <= 2);
    auto queries = count->load();
    fixture.m_time_client.set(NOW + duration_from_string("00:00:01"));
    fixture.store({duration_from_string("01:00:00.500")});
    fixture.set_position(duration_from_string("01:00:00.500"));
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REPLAYING);
    wait_until([&] {
      return model->get_size() == 2;
    });
    REQUIRE(get_offsets(*model) == std::vector<time_duration>{
      duration_from_string("00:59:59.500"),
      duration_from_string("01:00:00.500")});
    REQUIRE(*count == queries + 1);
  }

  UI_TEST_CASE("reading_near_the_present_is_throttled") {
    auto fixture = Fixture(duration_from_string("00:59:59"));
    fixture.store({duration_from_string("00:59:59.500")});
    auto count = std::make_shared<std::atomic_int>(0);
    auto model = fixture.make_model(count);
    fixture.m_session->play();
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REPLAYING);
    fixture.set_position(duration_from_string("00:59:59.500"));
    wait_until([&] {
      return model->get_size() == 1;
    });
    auto queries = count->load();
    for(auto i = 1; i <= 4; ++i) {
      fixture.m_time_client.set(NOW + milliseconds(100 * i));
      fixture.set_position(
        duration_from_string("00:59:59.500") + milliseconds(100 * i));
    }
    wait_for(milliseconds(100));
    REQUIRE(*count == queries);
    fixture.m_time_client.set(NOW + duration_from_string("00:00:00.500"));
    fixture.set_position(NOW_OFFSET);
    wait_until([&] {
      return *count == queries + 1;
    });
  }

  UI_TEST_CASE("replaying_reads_again_after_half_the_read_ahead_is_used") {
    auto fixture = Fixture(seconds(0));
    auto offsets = std::vector<time_duration>();
    for(auto i = 1; i <= 60; ++i) {
      offsets.push_back(seconds(i));
    }
    fixture.store(offsets);
    auto count = std::make_shared<std::atomic_int>(0);
    auto model = fixture.make_model(count);
    fixture.m_session->play();
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REPLAYING);
    fixture.set_position(seconds(1));
    wait_until([&] {
      return model->get_size() == 1;
    });
    auto queries = count->load();
    for(auto position = duration_from_string("00:00:01.100");
        position <= seconds(15); position += milliseconds(100)) {
      fixture.set_position(position);
    }
    REQUIRE(model->get_size() == 15);
    REQUIRE(*count == queries);
    fixture.set_position(duration_from_string("00:00:15.100"));
    wait_until([&] {
      return *count == queries + 1;
    });
  }

  UI_TEST_CASE("raising_the_speed_reads_further_ahead") {
    auto fixture = Fixture(seconds(0));
    auto offsets = std::vector<time_duration>();
    for(auto i = 1; i <= 400; ++i) {
      offsets.push_back(seconds(i));
    }
    fixture.store(offsets);
    auto count = std::make_shared<std::atomic_int>(0);
    auto model = fixture.make_model(count);
    fixture.m_session->play();
    fixture.set_position(seconds(1));
    wait_until([&] {
      return model->get_size() == 1;
    });
    auto queries = count->load();
    fixture.m_session->get_speed()->set(Decimal(10));
    wait_until([&] {
      return *count == queries + 1;
    });
    for(auto i = 1; i <= 3; ++i) {
      fixture.set_position(seconds(1 + 10 * i));
    }
    wait_until([&] {
      return model->get_size() == 31;
    });
    for(auto i = 4; i <= 15; ++i) {
      fixture.set_position(seconds(1 + 10 * i));
    }
    REQUIRE(model->get_size() == 151);
    REQUIRE(*count == queries + 1);
  }

  UI_TEST_CASE("a_higher_speed_raises_the_jump_threshold") {
    auto fixture = Fixture(seconds(0));
    auto offsets = std::vector<time_duration>();
    for(auto i = 1; i <= 60; ++i) {
      offsets.push_back(seconds(i));
    }
    fixture.store(offsets);
    auto count = std::make_shared<std::atomic_int>(0);
    auto model = fixture.make_model(count);
    fixture.m_session->get_speed()->set(Decimal(10));
    fixture.m_session->play();
    fixture.set_position(seconds(1));
    wait_until([&] {
      return model->get_size() == 1;
    });
    auto queries = count->load();
    fixture.set_position(seconds(16));
    REQUIRE(model->get_size() == 16);
    wait_for(milliseconds(200));
    REQUIRE(*count == queries);
  }

  UI_TEST_CASE("load_older_excludes_time_and_sales_after_a_seek_back") {
    auto fixture = Fixture(seconds(10));
    fixture.store({seconds(1), seconds(2), seconds(5)});
    auto model = fixture.make_model();
    REQUIRE(model->get_size() == 0);
    auto older = model->load_older(10);
    fixture.set_position(seconds(3));
    wait(std::move(older));
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{seconds(1), seconds(2)});
  }

  UI_TEST_CASE("a_burst_larger_than_a_read_is_shown_when_paused") {
    auto fixture = Fixture(seconds(4));
    fixture.store(std::vector<time_duration>(1500, seconds(5)));
    auto model = fixture.make_model();
    REQUIRE(model->get_size() == 0);
    fixture.set_position(seconds(5));
    wait_until([&] {
      return model->get_size() == 1500;
    });
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>(1500, seconds(5)));
  }

  UI_TEST_CASE("late_time_and_sales_are_read_when_the_list_is_empty") {
    auto fixture = Fixture(duration_from_string("00:59:59"));
    auto model = fixture.make_model();
    fixture.m_session->play();
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REPLAYING);
    wait_for(milliseconds(100));
    REQUIRE(model->get_size() == 0);
    fixture.store({duration_from_string("00:59:59.500")});
    fixture.m_time_client.set(NOW + duration_from_string("00:00:01"));
    fixture.set_position(NOW_OFFSET);
    wait_until([&] {
      return model->get_size() == 1;
    });
    REQUIRE(get_offsets(*model) ==
      std::vector<time_duration>{duration_from_string("00:59:59.500")});
  }

  UI_TEST_CASE("catching_up_to_real_time_classifies_by_the_bbo_at_each_trade") {
    auto fixture = Fixture(duration_from_string("00:59:59"));
    fixture.store_bbo(duration_from_string("00:59:50"),
      Money::ONE - Money::CENT, Money::ONE + Money::CENT);
    fixture.store({duration_from_string("00:59:59.200")});
    auto model = fixture.make_model();
    fixture.m_session->play();
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REPLAYING);
    fixture.set_position(duration_from_string("00:59:59.300"));
    wait_until([&] {
      return model->get_size() == 1;
    });
    fixture.store({duration_from_string("00:59:59.500")});
    fixture.store_bbo(duration_from_string("00:59:59.800"), Money::ONE,
      Money::ONE + 2 * Money::CENT);
    fixture.set_position(NOW_OFFSET);
    REQUIRE(fixture.m_session->get_state()->get() ==
      PlaybackState::REAL_TIME);
    wait_until([&] {
      return model->get_size() == 2;
    });
    REQUIRE(model->get(1).m_indicator == BboIndicator::INSIDE);
  }
}
