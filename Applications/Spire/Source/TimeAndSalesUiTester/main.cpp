#include <QApplication>
#include <QGridLayout>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/TimeAndSales/BboIndicator.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSalesUiTester/DemoTimeAndSalesModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  std::shared_ptr<ComboBox::QueryModel> make_security_query_model() {
    auto security_infos = std::vector<SecurityInfo>();
    security_infos.emplace_back(*ParseWildCardSecurity("MRU.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
      "Metro Inc.", "", 0);
    security_infos.emplace_back(*ParseWildCardSecurity("MG.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
      "Magna International Inc.", "", 0);
    security_infos.emplace_back(*ParseWildCardSecurity("MGA.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
      "Mega Uranium Ltd.", "", 0);
    security_infos.emplace_back(*ParseWildCardSecurity("MGAB.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
      "Mackenzie Global Fixed Income Alloc ETF", "", 0);
    security_infos.emplace_back(*ParseWildCardSecurity("MON.NYSE",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
      "Monsanto Co.", "", 0);
    security_infos.emplace_back(*ParseWildCardSecurity("MFC.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
      "Manulife Financial Corporation", "", 0);
    security_infos.emplace_back(*ParseWildCardSecurity("MX.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
      "Methanex Corporation", "", 0);
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto security_info : security_infos) {
      model->add(
        displayText(security_info.m_security).toLower(), security_info);
      model->add(
        QString::fromStdString(security_info.m_name).toLower(), security_info);
    }
    return model;
  }

  auto displayText(BboIndicator indicator) {
    if(indicator == BboIndicator::ABOVE_ASK) {
      static const auto value = QObject::tr("Above Ask");
      return value;
    } else if(indicator == BboIndicator::AT_ASK) {
      static const auto value = QObject::tr("At Ask");
      return value;
    } else if(indicator == BboIndicator::INSIDE) {
      static const auto value = QObject::tr("Inside");
      return value;
    } else if(indicator == BboIndicator::AT_BID) {
      static const auto value = QObject::tr("At Bid");
      return value;
    } else if(indicator == BboIndicator::BELOW_BID) {
      static const auto value = QObject::tr("Below Bid");
      return value;
    } else {
      static const auto value = QObject::tr("None");
      return value;
    }
  }

  auto get_bbo_indicator_list() {
    auto indicators = std::make_shared<ArrayListModel<QString>>();
    indicators->push(displayText(BboIndicator::UNKNOWN));
    indicators->push(displayText(BboIndicator::ABOVE_ASK));
    indicators->push(displayText(BboIndicator::AT_ASK));
    indicators->push(displayText(BboIndicator::INSIDE));
    indicators->push(displayText(BboIndicator::AT_BID));
    indicators->push(displayText(BboIndicator::BELOW_BID));
    return indicators;
  }

  struct TimeAndSalesTestWindow : QWidget {
    std::shared_ptr<DemoTimeAndSalesModel> m_time_and_sales;
    MoneyBox* m_money_box;
    DropDownBox* m_indicator_box;
    IntegerBox* m_loading_time_box;

    TimeAndSalesTestWindow(
        std::shared_ptr<DemoTimeAndSalesModel> time_and_sales,
        QWidget* parent = nullptr)
        : m_time_and_sales(std::move(time_and_sales)) {
      auto& markets = GetDefaultMarketDatabase();
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
      auto box = new MoneyBox(model, QHash<Qt::KeyboardModifier, Money>(
        {{Qt::NoModifier, Money::CENT}, {Qt::AltModifier, 5 * Money::CENT},
        {Qt::ControlModifier, 10 * Money::CENT},
        {Qt::ShiftModifier, 20 * Money::CENT}}));
      box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      model->connect_update_signal([=] (const auto& value) {
        if(value) {
          m_time_and_sales->set_price(*value);
        }
      });
      return box;
    }

    DropDownBox* make_indicator_box() {
      auto box = new DropDownBox(get_bbo_indicator_list());
      box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      box->get_current()->set(static_cast<int>(
        m_time_and_sales->get_bbo_indicator()));
      box->get_current()->connect_update_signal([=] (auto value) {
        if(value) {
          m_time_and_sales->set_bbo_indicator(
            static_cast<BboIndicator>(*value));
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
      auto check_box = new CheckBox();
      check_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      check_box->get_current()->connect_update_signal(
        [=] (bool checked) {
          if(checked) {
            m_money_box->setEnabled(false);
            m_indicator_box->setEnabled(false);
            m_time_and_sales->set_data_random(true);
          } else {
            m_time_and_sales->set_data_random(false);
            m_money_box->setEnabled(true);
            m_indicator_box->setEnabled(true);
            m_time_and_sales->set_price(*m_money_box->get_current()->get());
            m_time_and_sales->set_bbo_indicator(static_cast<BboIndicator>(
              *m_indicator_box->get_current()->get()));
          }
      });
      return check_box;
    }
  };

  struct TimeAndSalesStyleTestWindow : QWidget {
    std::shared_ptr<TimeAndSalesWindow> m_time_and_sales_window;
    BboIndicator m_indicator;
    TimeAndSalesWindowProperties::Styles m_style;
    DropDownBox* m_indicator_box;
    TextBox* m_text_color_box;
    TextBox* m_band_color_box;
    IntegerBox* m_font_size_box;

    TimeAndSalesStyleTestWindow(
        std::shared_ptr<TimeAndSalesWindow> time_and_sales_window)
        : m_time_and_sales_window(std::move(time_and_sales_window)) {
      m_text_color_box = make_color_box([=] (const auto& color) {
        m_style.m_text_color = color;
      });
      m_band_color_box = make_color_box([=] (const auto& color) {
        m_style.m_band_color = color;
      });
      m_font_size_box = make_font_size_box();
      m_indicator_box = new DropDownBox(get_bbo_indicator_list());
      m_indicator_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      m_indicator_box->get_current()->connect_update_signal(
        std::bind_front(&TimeAndSalesStyleTestWindow::on_indicator_current,
          this));
      m_indicator_box->get_current()->set(
        static_cast<int>(BboIndicator::UNKNOWN));
      auto layout = make_grid_layout(this);
      layout->setContentsMargins({scale_width(15), 0, scale_width(15), 0});
      layout->setHorizontalSpacing(scale_width(30));
      layout->addWidget(make_label(tr("Price Range:")), 0, 0);
      layout->addWidget(m_indicator_box, 0, 1);
      layout->addWidget(make_label(tr("Text Color:")), 1, 0);
      layout->addWidget(m_text_color_box, 1, 1);
      layout->addWidget(make_label(tr("Band Color:")), 2, 0);
      layout->addWidget(m_band_color_box, 2, 1);
      layout->addWidget(make_label(tr("Font Size:")), 3, 0);
      layout->addWidget(m_font_size_box, 3, 1);
      setFixedSize(scale(350, 180));
    }

    TextBox* make_color_box(std::function<void(const QColor& color)> update) {
      auto box = new TextBox();
      box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      box->get_current()->connect_update_signal([=] (auto value) {
        auto color = QColor(value);
        if(!color.isValid()) {
          return;
        }
        update(color);
        update_time_and_sales_style();
      });
      return box;
    }

    IntegerBox* make_font_size_box() {
      auto model = std::make_shared<LocalOptionalIntegerModel>();
      model->set_minimum(1);
      model->set_maximum(50);
      auto box = new IntegerBox(model);
      box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      box->get_current()->connect_update_signal([=] (auto value) {
        if(!value) {
          return;
        }
        m_style.m_font.setPixelSize(scale_width(*value));
        update_time_and_sales_style();
      });
      return box;
    }

    void update_time_and_sales_style() {
      const_cast<TimeAndSalesWindowProperties&>(
        m_time_and_sales_window->get_properties()).set_style(m_indicator,
          m_style);
      auto selector = [&] () -> Selector {
        switch(m_indicator) {
          case BboIndicator::ABOVE_ASK:
            return AboveAskRow();
          case BboIndicator::AT_ASK:
            return AtAskRow();
          case BboIndicator::INSIDE:
            return InsideRow();
          case BboIndicator::AT_BID:
            return AtBidRow();
          case BboIndicator::BELOW_BID:
            return BelowBidRow();
          default:
            return UnknownRow();
        }
      }();
      update_style(*m_time_and_sales_window, [&] (auto& style) {
        style.get(Any() > selector).
          set(BackgroundColor(m_style.m_band_color));
        style.get(Any() > selector > is_a<TextBox>()).
          set(text_style(m_style.m_font, m_style.m_text_color));
      });
    }

    void on_indicator_current(const boost::optional<int>& value) {
      if(!value) {
        return;
      }
      m_indicator = static_cast<BboIndicator>(*value);
      m_style = m_time_and_sales_window->get_properties().get_style(m_indicator);
      m_text_color_box->get_current()->set(m_style.m_text_color.name());
      m_band_color_box->get_current()->set(m_style.m_band_color.name());
      m_font_size_box->get_current()->set(
        unscale_width(m_style.m_font.pixelSize()));
    }
  };

  struct TimeAndSalesWindowController {
    std::shared_ptr<DemoTimeAndSalesModel> m_time_and_sales;
    std::shared_ptr<TimeAndSalesWindow> m_time_and_sales_window;
    std::unique_ptr<TimeAndSalesTestWindow> m_time_and_sales_test_window;
    std::unique_ptr<TimeAndSalesStyleTestWindow>
      m_time_and_sales_style_test_window;

    TimeAndSalesWindowController() {
      m_time_and_sales = std::make_shared<DemoTimeAndSalesModel>(Security());
      m_time_and_sales_window =
        std::make_shared<TimeAndSalesWindow>(make_security_query_model(),
          TimeAndSalesWindowProperties(),
          std::bind_front(&TimeAndSalesWindowController::model_builder, this));
      m_time_and_sales_test_window =
        std::make_unique<TimeAndSalesTestWindow>(m_time_and_sales);
      m_time_and_sales_style_test_window =
        std::make_unique<TimeAndSalesStyleTestWindow>(m_time_and_sales_window);
      m_time_and_sales_window->show();
      m_time_and_sales_window->installEventFilter(
        m_time_and_sales_test_window.get());
      m_time_and_sales_test_window->setAttribute(Qt::WA_ShowWithoutActivating);
      m_time_and_sales_test_window->show();
      m_time_and_sales_test_window->move(m_time_and_sales_window->pos().x() +
          m_time_and_sales_window->frameGeometry().width() + scale_width(100),
        m_time_and_sales_window->pos().y());
      m_time_and_sales_style_test_window->setAttribute(
        Qt::WA_ShowWithoutActivating);
      m_time_and_sales_style_test_window->show();
      m_time_and_sales_style_test_window->move(
        m_time_and_sales_test_window->pos().x(),
        m_time_and_sales_test_window->pos().y() +
          m_time_and_sales_test_window->frameGeometry().height());
    }

    std::shared_ptr<TimeAndSalesModel> model_builder(const Security& security) {
      auto price = m_time_and_sales->get_price();
      auto indicator = m_time_and_sales->get_bbo_indicator();
      auto period = m_time_and_sales->get_period();
      auto query_duration = m_time_and_sales->get_query_duration();
      auto is_data_random = m_time_and_sales->is_data_random();
      m_time_and_sales = std::make_shared<DemoTimeAndSalesModel>(security);
      m_time_and_sales->set_price(price);
      m_time_and_sales->set_bbo_indicator(indicator);
      m_time_and_sales->set_period(period);
      m_time_and_sales->set_query_duration(query_duration);
      m_time_and_sales->set_data_random(is_data_random);
      m_time_and_sales_test_window->m_time_and_sales = m_time_and_sales;
      return m_time_and_sales;
    }
  };
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("TimeAndSales Ui Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto controller = TimeAndSalesWindowController();
  application.exec();
}
