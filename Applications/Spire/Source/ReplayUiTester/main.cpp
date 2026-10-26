#include <vector>
#include <QApplication>
#include <QTextEdit>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/BookView/AggregateBookViewModel.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Playback/ReplayAttachmentMap.hpp"
#include "Spire/Playback/ReplayWindow.hpp"
#include "Spire/ReplayUiTester/DemoMarketDataFeed.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/PropertyHubMember.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto WINDOW_GAP = 30;
  const auto HISTORY = hours(4);
  const auto TIMELINE_DURATION = hours(8);
  const auto TIME_AND_SALE_INTERVAL = seconds(1);

  class UtcTimeClient {
    public:
      UtcTimeClient() = default;

      ptime get_time() {
        return microsec_clock::universal_time();
      }

      void close() {}
  };

  const auto& get_ticker_infos() {
    static const auto infos = [] {
      auto infos = std::vector<TickerInfo>();
      auto add_ticker = [&] (const std::string& ticker,
          const std::string& name) {
        if(auto parsed = parse_ticker(ticker)) {
          infos.push_back(TickerInfo(parsed, name, "", 0));
        }
      };
      add_ticker("ABX.TSX", "Barrick Gold Corporation");
      add_ticker("ARE.TSX", "Aecon Group Inc.");
      add_ticker("MRU.TSX", "Metro Inc.");
      add_ticker("MG.TSX", "Magna International Inc.");
      add_ticker("MGA.TSX", "Mega Uranium Ltd.");
      add_ticker("MGAB.TSX", "Mackenzie Global Fixed Income Alloc ETF");
      add_ticker("MON.NYSE", "Monsanto Co.");
      add_ticker("MFC.TSX", "Manulife Financial Corporation");
      add_ticker("MX.TSX", "Methanex Corporation");
      add_ticker("TD.TSX", "The Toronto-Dominion Bank");
      return infos;
    }();
    return infos;
  }

  auto to_text(const std::shared_ptr<PropertyHub>& hub,
      const ListModel<PropertyHubMember*>& roster) {
    auto is_member = [&] (auto member) {
      return member->get_hub()->get() == hub;
    };
    auto count = std::count_if(roster.begin(), roster.end(), is_member);
    auto ticker = hub->get<Ticker>(PropertyHub::TICKER_PROPERTY)->get();
    auto name = [&] {
      if(count == 1) {
        return (*std::find_if(roster.begin(), roster.end(), is_member))->
          get_name();
      }
      auto label = [&] {
        if(ticker) {
          return Spire::to_text(ticker);
        }
        return QObject::tr("Unassigned");
      }();
      if(count == 0) {
        return label;
      }
      return QString("%1 (%2)").arg(label).arg(count);
    }();
    return QString("%1 [%2]").arg(name).
      arg(QString::fromStdString(to_string(hub->get_id())).left(8));
  }

  auto to_state_text(PlaybackState state) {
    if(state == PlaybackState::PAUSED) {
      return QObject::tr("Paused");
    } else if(state == PlaybackState::REPLAYING) {
      return QObject::tr("Replaying");
    }
    return QObject::tr("Real Time");
  }

  struct SpecialProfile {
    DemoMarketDataFeed::Profile m_profile;
    QString m_label;
  };

  const auto& get_special_profiles() {
    using Pattern = DemoMarketDataFeed::Pattern;
    static const auto profiles = std::vector<SpecialProfile>{
      {{parse_ticker("TD.TSX"), Pattern::FIXED, milliseconds(50)},
        QObject::tr("High volume")},
      {{parse_ticker("MFC.TSX"), Pattern::FIXED, seconds(10)},
        QObject::tr("Low volume")},
      {{parse_ticker("MRU.TSX"), Pattern::BURST, TIME_AND_SALE_INTERVAL},
        QObject::tr("Bursts")},
      {{parse_ticker("ARE.TSX"), Pattern::LARGE_BURST, TIME_AND_SALE_INTERVAL},
        QObject::tr("Large bursts")},
      {{parse_ticker("MX.TSX"), Pattern::GAP, TIME_AND_SALE_INTERVAL},
        QObject::tr("Quiet periods")},
      {{parse_ticker("MG.TSX"), Pattern::LATE_START, TIME_AND_SALE_INTERVAL},
        QObject::tr("Late open")}};
    return profiles;
  }

  auto get_profiles() {
    auto profiles = std::vector<DemoMarketDataFeed::Profile>();
    for(auto& info : get_ticker_infos()) {
      auto special = std::find_if(get_special_profiles().begin(),
        get_special_profiles().end(), [&] (const auto& special) {
          return special.m_profile.m_ticker == info.m_ticker;
        });
      if(special == get_special_profiles().end()) {
        profiles.push_back(DemoMarketDataFeed::Profile{info.m_ticker,
          DemoMarketDataFeed::Pattern::FIXED, TIME_AND_SALE_INTERVAL});
      } else {
        profiles.push_back(special->m_profile);
      }
    }
    return profiles;
  }

  auto populate_tickers() {
    auto model = std::make_shared<LocalQueryModel<TickerInfo>>();
    for(auto& info : get_ticker_infos()) {
      model->add(Spire::to_text(info.m_ticker).toLower(), info);
      model->add(QString::fromStdString(info.m_name).toLower(), info);
    }
    return model;
  }
}

struct ReplayUiTester : QWidget {
  struct Replay {
    QString m_name;
    std::shared_ptr<ReplaySession> m_session;
    std::shared_ptr<PropertyHubListModel> m_attachments;
    scoped_connection m_attachments_connection;
    scoped_connection m_state_connection;
    scoped_connection m_speed_connection;
    scoped_connection m_timeline_connection;
  };

  struct MemberSession {
    PropertyHubMember* m_member;
    std::shared_ptr<ReplaySessionModel> m_session;
    scoped_connection m_connection;
  };

  ptime m_start;
  TestEnvironment m_environment;
  DemoMarketDataFeed m_market_data_feed;
  Clients m_clients;
  UserProfile m_user_profile;
  std::shared_ptr<TickerInfoQueryModel> m_tickers;
  std::shared_ptr<KeyBindingsModel> m_key_bindings;
  std::shared_ptr<TimeAndSalesPropertiesWindowFactory>
    m_time_and_sales_factory;
  std::shared_ptr<BookViewPropertiesWindowFactory> m_book_view_factory;
  std::shared_ptr<ReplayAttachmentMap> m_attachment_map;
  std::vector<std::unique_ptr<Replay>> m_replays;
  std::vector<std::unique_ptr<MemberSession>> m_member_sessions;
  QTextEdit* m_event_log;
  int m_replay_count;
  int m_time_and_sale_count;
  int m_book_view_count;
  scoped_connection m_attachment_map_connection;
  scoped_connection m_roster_connection;
  scoped_connection m_large_burst_connection;

  ReplayUiTester()
      : m_start(second_clock::universal_time() - HISTORY),
        m_market_data_feed(Ref(m_environment), get_profiles(), m_start),
        m_clients(std::in_place_type<TestClients>, Ref(m_environment)),
        m_user_profile("", false, false, {}, {},
          get_default_additional_tag_database(), {}, {}, {}, m_clients),
        m_tickers(populate_tickers()),
        m_key_bindings(std::make_shared<KeyBindingsModel>()),
        m_time_and_sales_factory(
          std::make_shared<TimeAndSalesPropertiesWindowFactory>()),
        m_book_view_factory(
          std::make_shared<BookViewPropertiesWindowFactory>()),
        m_attachment_map(m_user_profile.GetReplayAttachmentMap()),
        m_replay_count(0),
        m_time_and_sale_count(0),
        m_book_view_count(0) {
    setWindowTitle(tr("Replay Ui Tester"));
    setAttribute(Qt::WA_ShowWithoutActivating);
    auto& settings = m_user_profile.GetBlotterSettings();
    settings.SetActiveBlotter(settings.GetConsolidatedBlotter());
    auto layout = make_vbox_layout(this);
    m_event_log = new QTextEdit();
    m_event_log->setReadOnly(true);
    layout->addWidget(m_event_log);
    m_event_log->append(tr("Market data for testing:"));
    for(auto& special : get_special_profiles()) {
      m_event_log->append(tr("  %1  %2: %3").
        arg(Spire::to_text(special.m_profile.m_ticker)).
        arg(special.m_label).
        arg(DemoMarketDataFeed::describe(special.m_profile)));
    }
    m_event_log->append(tr("  All other tickers: %1").arg(
      DemoMarketDataFeed::describe(DemoMarketDataFeed::Profile{Ticker(),
        DemoMarketDataFeed::Pattern::FIXED, TIME_AND_SALE_INTERVAL})));
    for(auto& burst : m_market_data_feed.get_next_large_bursts()) {
      m_event_log->append(tr("%1: Next burst of %2 trades at %3.").
        arg(Spire::to_text(burst.m_ticker)).arg(burst.m_size).
        arg(Spire::to_text(burst.m_timestamp)));
    }
    m_large_burst_connection = m_market_data_feed.connect_large_burst_signal(
      std::bind_front(&ReplayUiTester::on_large_burst, this));
    auto open_replay_button = make_label_button(tr("Open Replay Window"));
    open_replay_button->connect_click_signal(
      std::bind_front(&ReplayUiTester::open_replay_window, this));
    auto open_time_and_sales_button =
      make_label_button(tr("Open Time and Sales Window"));
    open_time_and_sales_button->connect_click_signal(
      std::bind_front(&ReplayUiTester::open_time_and_sales_window, this));
    auto open_book_view_button = make_label_button(tr("Open Book View Window"));
    open_book_view_button->connect_click_signal(
      std::bind_front(&ReplayUiTester::open_book_view_window, this));
    auto button_layout = make_hbox_layout();
    button_layout->setSpacing(scale_width(8));
    button_layout->addWidget(open_book_view_button);
    button_layout->addWidget(open_replay_button);
    button_layout->addWidget(open_time_and_sales_button);
    layout->addLayout(button_layout);
    resize(scale(500, 400));
    auto& roster = *m_user_profile.GetPropertyHubMembers();
    for(auto member : roster) {
      add_member_session(*member);
    }
    m_roster_connection = roster.connect_operation_signal(
      std::bind_front(&ReplayUiTester::on_roster_operation, this));
    m_attachment_map_connection = m_attachment_map->connect_update_signal(
      std::bind_front(&ReplayUiTester::on_attachment_map_update, this));
  }

  void showEvent(QShowEvent* event) override {
    auto replay_window = open_replay_window();
    move(replay_window->pos() + QPoint((replay_window->frameGeometry().
      width() - frameGeometry().width()) / 2, 200));
    open_time_and_sales_window();
    open_book_view_window();
    QWidget::showEvent(event);
  }

  void closeEvent(QCloseEvent* event) override {
    QApplication::quit();
    QWidget::closeEvent(event);
  }

  ReplayWindow* open_replay_window() {
    ++m_replay_count;
    auto entry = std::make_unique<Replay>();
    auto replay = entry.get();
    replay->m_name = tr("Replay %1").arg(m_replay_count);
    replay->m_session = std::make_shared<ReplaySession>(
      Timeline(m_start, TIMELINE_DURATION), time_duration(0, 0, 0),
      Decimal(1), TimeClient(UtcTimeClient()));
    replay->m_attachments =
      make_attached_hub_list(m_attachment_map, replay->m_session);
    auto& session = *replay->m_session;
    replay->m_attachments_connection =
      replay->m_attachments->connect_operation_signal(
        [=] (const auto& operation) {
          on_attachments_operation(*replay, operation);
        });
    replay->m_state_connection =
      session.get_state()->connect_update_signal([=] (auto state) {
        on_state_update(*replay, state);
      });
    replay->m_speed_connection =
      session.get_speed()->connect_update_signal([=] (const auto& speed) {
        on_speed_update(*replay, speed);
      });
    replay->m_timeline_connection =
      session.get_timeline()->connect_update_signal([=] (const auto& timeline) {
        on_timeline_update(*replay, timeline);
      });
    auto window = new ReplayWindow(replay->m_session,
      m_user_profile.GetPropertyHubMembers(), replay->m_attachments,
      day_clock::universal_day() - years(1));
    window->setWindowTitle(replay->m_name);
    window->setAttribute(Qt::WA_DeleteOnClose);
    connect(window, &QObject::destroyed, this, [=] {
      remove_replay(*replay);
    });
    m_event_log->append(tr("%1 Opened").arg(replay->m_name));
    m_replays.push_back(std::move(entry));
    window->show();
    if(m_replay_count != 1) {
      auto offset = scale(WINDOW_GAP * (m_replay_count - 1),
        WINDOW_GAP * (m_replay_count - 1));
      window->move(QPoint(x() - (window->frameGeometry().width() -
        frameGeometry().width()) / 2, y() - 200) +
        QPoint(offset.width(), offset.height()));
    }
    return window;
  }

  void open_time_and_sales_window() {
    auto window = new TimeAndSalesWindow(Ref(m_user_profile), m_tickers,
      m_time_and_sales_factory, m_user_profile.GetTimeAndSalesModelBuilder());
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    auto offset = scale(WINDOW_GAP * m_time_and_sale_count,
      WINDOW_GAP * m_time_and_sale_count);
    window->move(QPoint(x() + frameGeometry().width() + scale_width(WINDOW_GAP),
      y()) + QPoint(offset.width(), offset.height()));
    ++m_time_and_sale_count;
  }

  void open_book_view_window() {
    auto window = new BookViewWindow(Ref(m_user_profile), m_tickers,
      m_key_bindings, m_book_view_factory,
      [] (const auto&) { return make_local_aggregate_book_view_model(); });
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    auto offset = scale(WINDOW_GAP * m_book_view_count,
      WINDOW_GAP * m_book_view_count);
    window->move(
      QPoint(x() - window->frameGeometry().width() - scale_width(WINDOW_GAP),
        y()) + QPoint(-offset.width(), offset.height()));
    ++m_book_view_count;
  }

  void remove_replay(Replay& replay) {
    m_event_log->append(tr("%1 Closed").arg(replay.m_name));
    std::erase_if(m_replays, [&] (const auto& entry) {
      return entry.get() == &replay;
    });
  }

  QString get_replay_name(const std::shared_ptr<ReplaySession>& session) const {
    if(!session) {
      return tr("None");
    }
    for(auto& replay : m_replays) {
      if(replay->m_session == session) {
        return replay->m_name;
      }
    }
    return tr("Unknown");
  }

  void on_attachments_operation(
      Replay& replay, const PropertyHubListModel::Operation& operation) {
    auto& roster = *m_user_profile.GetPropertyHubMembers();
    visit(operation,
      [&] (const PropertyHubListModel::AddOperation& operation) {
        m_event_log->append(tr("%1 Attached: %2").arg(replay.m_name).
          arg(to_text(replay.m_attachments->get(operation.m_index), roster)));
      },
      [&] (const PropertyHubListModel::PreRemoveOperation& operation) {
        m_event_log->append(tr("%1 Detached: %2").arg(replay.m_name).
          arg(to_text(replay.m_attachments->get(operation.m_index), roster)));
      });
  }

  void on_state_update(Replay& replay, PlaybackState state) {
    m_event_log->append(tr("%1 State: %2").arg(replay.m_name).
      arg(to_state_text(state)));
  }

  void on_speed_update(Replay& replay, const Decimal& speed) {
    m_event_log->append(tr("%1 Speed: %2x").arg(replay.m_name).
      arg(QString::fromStdString(speed.str())));
  }

  void on_timeline_update(Replay& replay, const Timeline& timeline) {
    m_event_log->append(tr("%1 Timeline: %2, Duration: %3").
      arg(replay.m_name).arg(Spire::to_text(timeline.m_start)).
      arg(Spire::to_text(timeline.m_duration)));
  }

  void on_large_burst(const DemoMarketDataFeed::Burst& burst,
      const DemoMarketDataFeed::Burst& next) {
    m_event_log->append(
      tr("%1: Burst of %2 trades at %3. Next burst of %4 trades at %5.").
        arg(Spire::to_text(burst.m_ticker)).arg(burst.m_size).
        arg(Spire::to_text(burst.m_timestamp)).arg(next.m_size).
        arg(Spire::to_text(next.m_timestamp)));
  }

  void on_attachment_map_update(const std::shared_ptr<PropertyHub>& hub,
      const std::shared_ptr<ReplaySession>& session) {
    m_event_log->append(tr("Attachment Map: %1 -> %2").
      arg(to_text(hub, *m_user_profile.GetPropertyHubMembers())).
      arg(get_replay_name(session)));
  }

  void add_member_session(PropertyHubMember& member) {
    auto entry = std::make_unique<MemberSession>();
    entry->m_member = &member;
    entry->m_session = make_attached_session_model(member, m_attachment_map);
    entry->m_connection = entry->m_session->connect_update_signal(
      [=, &member] (const auto& session) {
        m_event_log->append(tr("Member Session: %1 follows %2").
          arg(member.get_name()).arg(get_replay_name(session)));
      });
    if(auto& session = entry->m_session->get()) {
      m_event_log->append(tr("Member Session: %1 follows %2").
        arg(member.get_name()).arg(get_replay_name(session)));
    }
    m_member_sessions.push_back(std::move(entry));
  }

  void on_roster_operation(
      const ListModel<PropertyHubMember*>::Operation& operation) {
    auto& roster = *m_user_profile.GetPropertyHubMembers();
    visit(operation,
      [&] (const ListModel<PropertyHubMember*>::AddOperation& operation) {
        add_member_session(*roster.get(operation.m_index));
      },
      [&] (const ListModel<PropertyHubMember*>::PreRemoveOperation&
          operation) {
        auto member = roster.get(operation.m_index);
        std::erase_if(m_member_sessions, [&] (const auto& entry) {
          return entry->m_member == member;
        });
      });
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Replay Ui Tester"));
  initialize_resources();
  auto tester = ReplayUiTester();
  tester.show();
  application.exec();
}
