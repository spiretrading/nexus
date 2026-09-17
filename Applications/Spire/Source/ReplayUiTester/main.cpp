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
#include "Spire/Playback/ReplayWindow.hpp"
#include "Spire/ReplayUiTester/GeneratedTimeAndSalesModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
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
  const auto TIME_AND_SALE_PERIOD = seconds(1);

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
      auto label = ticker ? Spire::to_text(ticker) : QObject::tr("Unassigned");
      if(count == 0) {
        return label;
      }
      return QString("%1 (%2)").arg(label).arg(count);
    }();
    return QString("%1 [%2]").arg(name).
      arg(QString::fromStdString(to_string(hub->get_id())).left(8));
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
  ptime m_start;
  TestEnvironment m_environment;
  Clients m_clients;
  UserProfile m_user_profile;
  std::shared_ptr<TickerInfoQueryModel> m_tickers;
  std::shared_ptr<KeyBindingsModel> m_key_bindings;
  std::shared_ptr<TimeAndSalesPropertiesWindowFactory>
    m_time_and_sales_factory;
  std::shared_ptr<BookViewPropertiesWindowFactory> m_book_view_factory;
  std::shared_ptr<ArrayListModel<std::shared_ptr<PropertyHub>>> m_attachments;
  ReplayWindow m_replay_window;
  QTextEdit* m_event_log;
  int m_time_and_sale_count;
  int m_book_view_count;
  scoped_connection m_attachments_connection;

  ReplayUiTester()
      : m_start(microsec_clock::universal_time() - HISTORY),
        m_clients(std::in_place_type<TestClients>, Ref(m_environment)),
        m_user_profile("", false, false, {}, {},
          get_default_additional_tag_database(), {}, {}, {}, m_clients),
        m_tickers(populate_tickers()),
        m_key_bindings(std::make_shared<KeyBindingsModel>()),
        m_time_and_sales_factory(
          std::make_shared<TimeAndSalesPropertiesWindowFactory>()),
        m_book_view_factory(
          std::make_shared<BookViewPropertiesWindowFactory>()),
        m_attachments(std::make_shared<
          ArrayListModel<std::shared_ptr<PropertyHub>>>()),
        m_replay_window(std::make_shared<LocalTimelineModel>(
          Timeline(m_start, TIMELINE_DURATION)),
          TimeClient(UtcTimeClient()),
          std::make_shared<LocalDurationModel>(time_duration(0, 0, 0)),
          m_user_profile.GetPropertyHubMembers(), m_attachments,
          std::make_shared<LocalPlaybackSpeedModel>(1),
          day_clock::universal_day() - years(1)),
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
    button_layout->addWidget(open_time_and_sales_button);
    layout->addLayout(button_layout);
    resize(scale(500, 400));
    m_attachments_connection = m_attachments->connect_operation_signal(
      std::bind_front(&ReplayUiTester::on_attachments_operation, this));
    m_replay_window.installEventFilter(this);
  }

  bool eventFilter(QObject* object, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      QApplication::quit();
    }
    return QWidget::eventFilter(object, event);
  }

  void showEvent(QShowEvent* event) override {
    m_replay_window.show();
    move(m_replay_window.pos() + QPoint((m_replay_window.frameGeometry().
      width() - frameGeometry().width()) / 2, 200));
    open_time_and_sales_window();
    open_book_view_window();
    QWidget::showEvent(event);
  }

  void closeEvent(QCloseEvent* event) override {
    QApplication::quit();
    QWidget::closeEvent(event);
  }

  std::shared_ptr<TimeAndSalesModel> time_and_sales_model_builder(
      const Ticker& ticker) {
    return std::make_shared<GeneratedTimeAndSalesModel>(
      ticker, m_start, TIME_AND_SALE_PERIOD);
  }

  void open_time_and_sales_window() {
    auto window = new TimeAndSalesWindow(Ref(m_user_profile), m_tickers,
      m_time_and_sales_factory,
      std::bind_front(&ReplayUiTester::time_and_sales_model_builder, this));
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

  void on_attachments_operation(
      const PropertyHubListModel::Operation& operation) {
    visit(operation,
      [&] (const PropertyHubListModel::AddOperation& operation) {
        m_event_log->append(tr("Attached: %1").
          arg(to_text(m_attachments->get(operation.m_index),
            *m_user_profile.GetPropertyHubMembers())));
      },
      [&] (const PropertyHubListModel::PreRemoveOperation& operation) {
        m_event_log->append(tr("Detached: %1").
          arg(to_text(m_attachments->get(operation.m_index),
            *m_user_profile.GetPropertyHubMembers())));
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
