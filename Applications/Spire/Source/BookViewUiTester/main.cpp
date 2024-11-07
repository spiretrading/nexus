#include <QApplication>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGroupBox>
#include <QTextEdit>
#include "Spire/BookView/BboBox.hpp"
#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

template<typename M, typename U>
MoneyBox* make_money_box(M model, U field) {
  return new MoneyBox(
    std::make_shared<OptionalScalarValueModelDecorator<Money>>(
      std::make_shared<ScalarValueModelDecorator<Money>>(
        make_field_value_model(model, field))));
}

template<typename M, typename U>
QuantityBox* make_quantity_box(M model, U field) {
  return new QuantityBox(
    std::make_shared<OptionalScalarValueModelDecorator<Quantity>>(
      std::make_shared<ScalarValueModelDecorator<Quantity>>(
        make_field_value_model(model, field))));
}

struct PropertiesTester {
  BookViewLevelPropertiesPage m_page;
  QTextEdit m_logs;
  BookViewLevelProperties m_previous_properties;
  int m_line_number;

  PropertiesTester()
      : m_page(BookViewLevelPropertiesPage(
        std::make_shared<LocalLevelPropertiesModel>(
          BookViewLevelProperties::get_default()))),
        m_logs(QTextEdit()),
        m_previous_properties(m_page.get_current()->get()),
        m_line_number(0) {
    m_page.show();
    m_page.resize(scale(360, 503));
    m_logs.show();
    m_logs.resize(scale(300, 500));
    m_logs.move(
      m_page.pos().x() + m_logs.frameGeometry().width() + scale_width(100),
      m_logs.pos().y());
    m_page.get_current()->connect_update_signal(
      std::bind_front(&PropertiesTester::on_properties_update, this));
  }

  void on_properties_update(const BookViewLevelProperties& properties) {
    auto log = QString();
    log += QString::number(++m_line_number) + ": ";
    if(m_previous_properties.m_font != properties.m_font) {
      log += QString("font{%1, %2, %3}").arg(properties.m_font.family()).
        arg(QFontDatabase().styleString(properties.m_font)).
        arg(unscale_width(properties.m_font.pixelSize()));
    } else if(m_previous_properties.m_is_grid_enabled !=
        properties.m_is_grid_enabled) {
      log += QString("grid_enabled{%1}").arg(properties.m_is_grid_enabled);
    } else {
      log += "color_scheme{ ";
      for(auto i = 0; i < std::ssize(properties.m_color_scheme); ++i) {
        log += QString("%1[%2]").arg(i + 1).
          arg(properties.m_color_scheme[i].name(QColor::HexArgb)) += " ";
      }
      log += "}";
    }
    m_logs.append(log);
    m_previous_properties = properties;
  }
};

struct BookViewTester : QWidget {
  BookViewTester(std::shared_ptr<SecurityTechnicalsModel> technicals,
      std::shared_ptr<ValueModel<BboQuote>> bbo_quote,
      std::shared_ptr<QuantityModel> default_bid_quantity,
      std::shared_ptr<QuantityModel> default_ask_quantity,
      std::shared_ptr<OptionalIntegerModel> font_size,
      QWidget* parent = nullptr)
      : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    auto technicals_group_box = new QGroupBox(tr("Technicals"));
    auto technicals_layout = new QFormLayout(technicals_group_box);
    technicals_layout->addRow(tr("High:"),
      make_money_box(technicals, &SecurityTechnicals::m_high));
    technicals_layout->addRow(tr("Low:"),
      make_money_box(technicals, &SecurityTechnicals::m_low));
    technicals_layout->addRow(tr("Open:"),
      make_money_box(technicals, &SecurityTechnicals::m_open));
    technicals_layout->addRow(tr("Close:"),
      make_money_box(technicals, &SecurityTechnicals::m_close));
    technicals_layout->addRow(tr("Volume:"),
      make_quantity_box(technicals, &SecurityTechnicals::m_volume));
    technicals_layout->addRow(tr("Default Bid Quantity:"), new QuantityBox(
      std::make_shared<OptionalScalarValueModelDecorator<Quantity>>(
        default_bid_quantity)));
    technicals_layout->addRow(tr("Default Ask Quantity:"), new QuantityBox(
      std::make_shared<OptionalScalarValueModelDecorator<Quantity>>(
        default_ask_quantity)));
    layout->addWidget(technicals_group_box);
    auto bbo_bid = make_field_value_model(bbo_quote, &BboQuote::m_bid);
    auto bbo_ask = make_field_value_model(bbo_quote, &BboQuote::m_ask);
    auto bbo_quote_group_box = new QGroupBox(tr("BBO Quote"));
    auto bbo_quote_layout = new QFormLayout(bbo_quote_group_box);
    bbo_quote_layout->addRow(tr("Bid Price:"),
      make_money_box(bbo_bid, &Quote::m_price));
    bbo_quote_layout->addRow(tr("Bid Quantity:"),
      make_quantity_box(bbo_bid, &Quote::m_size));
    bbo_quote_layout->addRow(tr("Ask Price:"),
      make_money_box(bbo_ask, &Quote::m_price));
    bbo_quote_layout->addRow(tr("Ask Quantity:"),
      make_quantity_box(bbo_ask, &Quote::m_size));
    layout->addWidget(bbo_quote_group_box);
    auto properties_group_box = new QGroupBox(tr("Properties"));
    auto properties_layout = new QFormLayout(properties_group_box);
    properties_layout->addRow(tr("Font Size:"),
      new IntegerBox(std::move(font_size)));
    layout->addWidget(properties_group_box);
    setFixedWidth(scale_width(350));
  }

  bool eventFilter(QObject* object, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      QApplication::quit();
    }
    return QWidget::eventFilter(object, event);
  }
};

struct MarketDepthContainer : QWidget {
  MarketDepthContainer(std::shared_ptr<ValueModel<BboQuote>> bbo_quote,
      std::shared_ptr<OptionalIntegerModel> font_size,
      QWidget* parent = nullptr)
      : QWidget(parent) {
    auto layout = make_hbox_layout(this);
    layout->setSpacing(scale_width(2));
    auto bid_panel = new QWidget();
    bid_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto bid_layout = make_vbox_layout(bid_panel);
    bid_layout->addWidget(
      new BboBox(make_field_value_model(bbo_quote, &BboQuote::m_bid)));
    bid_layout->addStretch(1);
    layout->addWidget(bid_panel, 1);
    auto ask_panel = new QWidget();
    ask_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto ask_layout = make_vbox_layout(ask_panel);
    ask_layout->addWidget(
      new BboBox(make_field_value_model(bbo_quote, &BboQuote::m_ask)));
    ask_layout->addStretch(1);
    layout->addWidget(ask_panel, 1);
    font_size->connect_update_signal([=] (const auto& size) {
      if(size) {
        update_style(*this, [&] (auto& style) {
          style.get(Any() > is_a<QWidget>() > is_a<BboBox>() > is_a<TextBox>()).
            set(FontSize(scale_width(*size)));
        });
      }
    });
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("BookView Ui Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto technicals = std::make_shared<LocalSecuirtyTechnicalsModel>(
    SecurityTechnicals(1100, Money(144.4), Money(142.11), Money(144.25),
      Money(144.99)));
  auto bbo_quote = std::make_shared<LocalValueModel<BboQuote>>(
    BboQuote(Quote(Money(143.53), 51, Side::BID),
      Quote(Money(143.54), 39, Side::ASK), second_clock::universal_time()));
  auto default_bid_quantity = std::make_shared<LocalQuantityModel>(100);
  auto default_ask_quantity = std::make_shared<LocalQuantityModel>(100);
  auto font_size = std::make_shared<LocalOptionalIntegerModel>(10);
  auto tester = BookViewTester(technicals, bbo_quote,
    default_bid_quantity, default_ask_quantity, font_size);
  auto widget = QWidget();
  auto layout = make_vbox_layout(&widget);
  auto panel = new TechnicalsPanel(technicals, default_bid_quantity,
    default_ask_quantity);
  layout->addWidget(panel);
  layout->addWidget(new MarketDepthContainer(bbo_quote, font_size));
  widget.installEventFilter(&tester);
  widget.show();
  tester.show();
  tester.move(
    tester.pos().x() + widget.frameGeometry().width() + scale_width(100),
    widget.pos().y());
  auto properties_tester = PropertiesTester();
  properties_tester.m_page.installEventFilter(&tester);
  application.exec();
}
