#include <vector>
#include <QApplication>
#include <QTextEdit>
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

  auto populate_tickers() {
    auto model = std::make_shared<LocalQueryModel<TickerInfo>>();
    for(auto& info : get_ticker_infos()) {
      model->add(to_text(info.m_ticker).toLower(), info);
      model->add(QString::fromStdString(info.m_name).toLower(), info);
    }
    return model;
  }

}

struct ReplayUiTester : QWidget {
  ptime m_start;
  std::shared_ptr<TickerInfoQueryModel> m_tickers;
  std::shared_ptr<TimeAndSalesPropertiesWindowFactory> m_factory;
  std::shared_ptr<ArrayListModel<SelectableTarget>> m_targets;
  ReplayWindow m_replay_window;
  QTextEdit* m_event_log;
  int m_time_and_sale_count;
  scoped_connection m_targets_connection;

  ReplayUiTester()
      : m_start(microsec_clock::universal_time() - HISTORY),
        m_tickers(populate_tickers()),
        m_factory(std::make_shared<TimeAndSalesPropertiesWindowFactory>()),
        m_targets(std::make_shared<ArrayListModel<SelectableTarget>>()),
        m_replay_window(std::make_shared<LocalTimelineModel>(
          Timeline(m_start, TIMELINE_DURATION)),
          TimeClient(UtcTimeClient()),
          std::make_shared<LocalDurationModel>(time_duration(0, 0, 0)),
          m_targets,
          std::make_shared<LocalPlaybackSpeedModel>(1),
          day_clock::universal_day() - years(1)),
        m_time_and_sale_count(0) {
    setWindowTitle(tr("Replay Ui Tester"));
    setAttribute(Qt::WA_ShowWithoutActivating);
    auto layout = make_vbox_layout(this);
    m_event_log = new QTextEdit();
    m_event_log->setReadOnly(true);
    layout->addWidget(m_event_log);
    auto open_button = make_label_button(tr("Open Time and Sales Window"));
    open_button->connect_click_signal(
      std::bind_front(&ReplayUiTester::open_time_and_sales_window, this));
    layout->addWidget(open_button);
    resize(scale(500, 400));
    m_targets_connection = m_targets->connect_operation_signal(
      std::bind_front(&ReplayUiTester::on_targets_operation, this));
  }

  void showEvent(QShowEvent* event) override {
    m_replay_window.show();
    move(m_replay_window.pos() + QPoint(-width() / 2, 200));
    open_time_and_sales_window();
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
    auto window = new TimeAndSalesWindow(m_tickers, m_factory,
      std::bind_front(&ReplayUiTester::time_and_sales_model_builder, this));
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    auto offset = scale(WINDOW_GAP * m_time_and_sale_count,
      WINDOW_GAP * m_time_and_sale_count);
    window->move(QPoint(x() + frameGeometry().width() + scale_width(WINDOW_GAP),
      y()) + QPoint(offset.width(), offset.height()));
    ++m_time_and_sale_count;
  }

  void on_targets_operation(
      const SelectableTargetListModel::Operation& operation) {
    auto& targets = *m_targets;
    visit(operation,
      [&] (const SelectableTargetListModel::AddOperation& operation) {
        m_event_log->append(QString("Add %1: %2").
          arg(operation.m_index).
          arg(to_text(targets.get(operation.m_index).m_target)));
      },
      [&] (const SelectableTargetListModel::UpdateOperation& operation) {
        m_event_log->append(QString("Update %1: %2").
          arg(operation.m_index).
          arg(to_text(operation.get_value().m_target)));
      },
      [&] (const SelectableTargetListModel::RemoveOperation& operation) {
        m_event_log->append(QString("Remove %1").arg(operation.m_index));
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
