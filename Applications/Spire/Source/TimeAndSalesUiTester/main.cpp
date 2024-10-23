#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalSecurityQueryModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSalesUiTester/DemoTimeAndSalesModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Version.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

std::shared_ptr<SecurityQueryModel> populate_securities() {
  auto security_infos = std::vector<SecurityInfo>();
  security_infos.emplace_back(ParseSecurity("MRU.TSX"), "Metro Inc.", "", 0);
  security_infos.emplace_back(ParseSecurity("MG.TSX"),
    "Magna International Inc.", "", 0);
  security_infos.emplace_back(ParseSecurity("MGA.TSX"),
    "Mega Uranium Ltd.", "", 0);
  security_infos.emplace_back(ParseSecurity("MGAB.TSX"),
    "Mackenzie Global Fixed Income Alloc ETF", "", 0);
  security_infos.emplace_back(ParseSecurity("MON.NYSE"), "Monsanto Co.", "", 0);
  security_infos.emplace_back(ParseSecurity("MFC.TSX"),
    "Manulife Financial Corporation", "", 0);
  security_infos.emplace_back(ParseSecurity("MX.TSX"),
    "Methanex Corporation", "", 0);
  auto model =
    std::make_shared<LocalSecurityQueryModel>(GetDefaultMarketDatabase());
  for(auto& security_info : security_infos) {
    model->add(security_info);
  }
  return model;
}

auto make_bbo_indicator_list() {
  auto indicators = std::make_shared<ArrayListModel<QString>>();
  indicators->push(to_text(BboIndicator::UNKNOWN));
  indicators->push(to_text(BboIndicator::ABOVE_ASK));
  indicators->push(to_text(BboIndicator::AT_ASK));
  indicators->push(to_text(BboIndicator::INSIDE));
  indicators->push(to_text(BboIndicator::AT_BID));
  indicators->push(to_text(BboIndicator::BELOW_BID));
  return indicators;
}

struct TimeAndSalesTestWindow : QWidget {
  std::shared_ptr<DemoTimeAndSalesModel> m_time_and_sales;
  MoneyBox* m_money_box;
  DropDownBox* m_indicator_box;
  IntegerBox* m_loading_time_box;

  explicit TimeAndSalesTestWindow(
      std::shared_ptr<DemoTimeAndSalesModel> time_and_sales,
      QWidget* parent = nullptr)
      : m_time_and_sales(std::move(time_and_sales)) {
    auto layout = make_grid_layout(this);
    layout->setContentsMargins({scale_width(15), 0, scale_width(15), 0});
    layout->setHorizontalSpacing(scale_width(30));
    layout->addWidget(make_label(tr("Price:")), 0, 0);
    m_money_box = make_money_box();
    layout->addWidget(m_money_box, 0, 1);
    layout->addWidget(make_label(tr("Price Range:")), 1, 0);
    m_indicator_box = make_indicator_box();
    layout->addWidget(m_indicator_box, 1, 1);
    layout->addWidget(make_label(tr("Period (ms):")), 2, 0);
    layout->addWidget(make_period_box(), 2, 1);
    layout->addWidget(make_label(tr("Loading Time (ms):")), 3, 0);
    m_loading_time_box = make_loading_time_box();
    layout->addWidget(m_loading_time_box, 3, 1);
    layout->addWidget(make_label(tr("All Data Loaded")), 4, 0);
    layout->addWidget(make_load_all_data_check_box(), 4, 1);
    layout->addWidget(make_label(tr("Random data")), 5, 0);
    layout->addWidget(make_random_check_box(), 5, 1);
    setFixedSize(scale(350, 250));
  }

  bool eventFilter(QObject* object, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      QApplication::quit();
    }
    return QWidget::eventFilter(object, event);
  }

  MoneyBox* make_money_box() {
    auto model = std::make_shared<LocalOptionalMoneyModel>(
      m_time_and_sales->get_price());
    model->set_minimum(Money(0));
    auto box = new MoneyBox(model);
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    model->connect_update_signal([=] (const auto& value) {
      if(value) {
        m_time_and_sales->set_price(*value);
      }
    });
    return box;
  }

  DropDownBox* make_indicator_box() {
    auto box = new DropDownBox(make_bbo_indicator_list());
    box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    box->get_current()->set(static_cast<int>(
      m_time_and_sales->get_bbo_indicator()));
    box->get_current()->connect_update_signal([=] (auto value) {
      if(value) {
        m_time_and_sales->set_bbo_indicator(static_cast<BboIndicator>(*value));
      }
    });
    return box;
  }

  IntegerBox* make_period_box() {
    auto model = std::make_shared<LocalOptionalIntegerModel>(
      m_time_and_sales->get_period().total_milliseconds());
    model->set_minimum(0);
    auto box = new IntegerBox(model);
    box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    model->connect_update_signal([=] (auto value) {
      if(value) {
        m_time_and_sales->set_period(milliseconds(*value));
      }
    });
    return box;
  }

  IntegerBox* make_loading_time_box() {
    auto model = std::make_shared<LocalOptionalIntegerModel>(
      m_time_and_sales->get_query_duration().total_milliseconds());
    model->set_minimum(0);
    auto box = new IntegerBox(model);
    box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    model->connect_update_signal([=] (auto value) {
      if(m_time_and_sales->get_query_duration() == pos_infin) {
        return;
      }
      if(value) {
        m_time_and_sales->set_query_duration(milliseconds(*value));
      }
    });
    return box;
  }

  CheckBox* make_load_all_data_check_box() {
    auto check_box = new CheckBox();
    check_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    check_box->get_current()->connect_update_signal([=] (auto checked) {
      if(checked) {
        m_time_and_sales->set_query_duration(pos_infin);
      } else if(auto current = m_loading_time_box->get_current()->get()) {
        m_time_and_sales->set_query_duration(milliseconds(*current));
      }
    });
    return check_box;
  }

  CheckBox* make_random_check_box() {
    auto check_box = new CheckBox(std::make_shared<LocalBooleanModel>());
    m_time_and_sales->set_data_random(check_box->get_current()->get());
    check_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    check_box->get_current()->connect_update_signal([=] (bool checked) {
      m_money_box->setEnabled(!checked);
      m_indicator_box->setEnabled(!checked);
      m_time_and_sales->set_data_random(checked);
    });
    check_box->get_current()->set(true);
    return check_box;
  }
};

struct TimeAndSalesWindowController {
  TimeAndSalesWindow m_time_and_sales_window;
  TimeAndSalesTestWindow m_time_and_sales_test_window;

  explicit TimeAndSalesWindowController(
      std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory)
      : m_time_and_sales_window(populate_securities(), std::move(factory),
          std::bind_front(&TimeAndSalesWindowController::model_builder, this)),
        m_time_and_sales_test_window(
          std::make_shared<DemoTimeAndSalesModel>()) {
    m_time_and_sales_window.show();
    m_time_and_sales_window.installEventFilter(&m_time_and_sales_test_window);
    m_time_and_sales_test_window.setAttribute(Qt::WA_ShowWithoutActivating);
    m_time_and_sales_test_window.show();
    m_time_and_sales_test_window.move(m_time_and_sales_window.pos().x() +
      m_time_and_sales_window.frameGeometry().width() + scale_width(100),
      m_time_and_sales_window.pos().y());
  }

  std::shared_ptr<TimeAndSalesModel> model_builder(const Security&) {
    auto time_and_sales = m_time_and_sales_test_window.m_time_and_sales;
    auto new_time_and_sales = std::make_shared<DemoTimeAndSalesModel>();
    new_time_and_sales->set_price(time_and_sales->get_price());
    new_time_and_sales->set_bbo_indicator(time_and_sales->get_bbo_indicator());
    new_time_and_sales->set_period(time_and_sales->get_period());
    new_time_and_sales->set_query_duration(
      time_and_sales->get_query_duration());
    new_time_and_sales->set_data_random(time_and_sales->is_data_random());
    m_time_and_sales_test_window.m_time_and_sales = new_time_and_sales;
    return new_time_and_sales;
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Time and Sales UI Tester"));
  initialize_resources();
  auto controller = TimeAndSalesWindowController(
    std::make_shared<TimeAndSalesPropertiesWindowFactory>());
  application.exec();
}
