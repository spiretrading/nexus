#include <QApplication>
#include <ranges>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGroupBox>
#include <QTextEdit>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Spire/BookView/BboBox.hpp"
#include "Spire/BookView/BookViewPropertiesWindow.hpp"
#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

const QString& to_text(Qt::KeyboardModifier modifier) {
  if(modifier == Qt::ShiftModifier) {
    static const auto value = QObject::tr("Shift");
    return value;
  } else if(modifier == Qt::ControlModifier) {
    static const auto value = QObject::tr("Control");
    return value;
  } else if(modifier == Qt::AltModifier) {
    static const auto value = QObject::tr("Alt");
    return value;
  }
  static const auto none = QString();
  return none;
}

void copy_interactions(const InteractionsKeyBindingsModel& from,
    InteractionsKeyBindingsModel& to) {
  to.get_default_quantity()->set(from.get_default_quantity()->get());
  for(auto i :
      std::views::iota(0, InteractionsKeyBindingsModel::MODIFIER_COUNT)) {
    auto modifier = to_modifier(i);
    to.get_quantity_increment(modifier)->set(
      from.get_quantity_increment(modifier)->get());
    to.get_price_increment(modifier)->set(
      from.get_price_increment(modifier)->get());
  }
  to.is_cancel_on_fill()->set(from.is_cancel_on_fill()->get());
}

std::shared_ptr<SecurityInfoQueryModel> populate_security_query_model() {
  auto security_infos = std::vector<SecurityInfo>();
  security_infos.emplace_back(ParseSecurity("MRU.TSX"),
    "Metro Inc.", "", 0);
  security_infos.emplace_back(ParseSecurity("MG.TSX"),
    "Magna International Inc.", "", 0);
  security_infos.emplace_back(ParseSecurity("MGA.TSX"),
    "Mega Uranium Ltd.", "", 0);
  security_infos.emplace_back(ParseSecurity("MGAB.TSX"),
    "Mackenzie Global Fixed Income Alloc ETF", "", 0);
  security_infos.emplace_back(ParseSecurity("MON.NYSE"),
    "Monsanto Co.", "", 0);
  security_infos.emplace_back(ParseSecurity("MFC.TSX"),
    "Manulife Financial Corporation", "", 0);
  security_infos.emplace_back(ParseSecurity("MX.TSX"),
    "Methanex Corporation", "", 0);
  auto model = std::make_shared<LocalQueryModel<SecurityInfo>>();
  for(auto& security_info : security_infos) {
    model->add(to_text(security_info.m_security).toLower(), security_info);
    model->add(
      QString::fromStdString(security_info.m_name).toLower(), security_info);
  }
  return model;
}

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
  std::shared_ptr<BookViewPropertiesModel> m_properties;
  std::shared_ptr<KeyBindingsModel> m_key_bindings;
  std::shared_ptr<SecurityModel> m_security;
  BookViewPropertiesWindow m_properties_window;
  BookViewProperties m_previous_properties;
  InteractionsKeyBindingsModel m_previous_interactions;
  QTextEdit m_logs;
  int m_line_number;
  connection m_properties_connection;
  connection m_security_connection;
  std::array<connection, 10> m_interaction_connections;

  PropertiesTester(std::shared_ptr<BookViewPropertiesModel> properties,
      std::shared_ptr<KeyBindingsModel> key_bindings,
      std::shared_ptr<SecurityModel> security)
      : m_properties(std::move(properties)),
        m_key_bindings(std::move(key_bindings)),
        m_security(std::move(security)),
        m_properties_window(BookViewPropertiesWindow(m_properties,
          m_key_bindings, m_security, GetDefaultMarketDatabase())),
        m_previous_properties(m_properties->get()),
        m_line_number(0) {
    on_security_update(m_security->get());
    m_properties_window.show();
    m_logs.show();
    m_logs.resize(scale(300, 500));
    m_logs.move(m_properties_window.pos().x() + m_logs.frameGeometry().width() +
      scale_width(100), m_logs.pos().y());
    m_properties_connection = m_properties->connect_update_signal(
      std::bind_front(&PropertiesTester::on_properties_update, this));
    m_security_connection = m_security->connect_update_signal(
      std::bind_front(&PropertiesTester::on_security_update, this));
  }

  void on_properties_update(const BookViewProperties& properties) {
    if(m_previous_properties.m_level_properties.m_font !=
        properties.m_level_properties.m_font) {
      auto log = QString();
      log += QString::number(++m_line_number) + QString(": font{%1, %2, %3}").
        arg(properties.m_level_properties.m_font.family()).
        arg(QFontDatabase().styleString(properties.m_level_properties.m_font)).
        arg(unscale_width(properties.m_level_properties.m_font.pixelSize()));
      m_logs.append(log);
    }
    if(m_previous_properties.m_level_properties.m_is_grid_enabled !=
        properties.m_level_properties.m_is_grid_enabled) {
      auto log = QString();
      log += QString::number(++m_line_number) + QString(": grid_enabled: %1").
        arg(properties.m_level_properties.m_is_grid_enabled);
      m_logs.append(log);
    }
    if(m_previous_properties.m_level_properties.m_color_scheme !=
        properties.m_level_properties.m_color_scheme) {
      auto log = QString();
      log += QString::number(++m_line_number) + ": color_scheme{ ";
      for(auto i = 0;
          i < std::ssize(properties.m_level_properties.m_color_scheme); ++i) {
        log += QString("%1[%2]").arg(i + 1).
          arg(properties.m_level_properties.m_color_scheme[i].name(
            QColor::HexArgb)) += " ";
      }
      log += "}";
      m_logs.append(log);
    }
    auto& previous_highlight = m_previous_properties.m_highlight_properties;
    auto& current_highlight = properties.m_highlight_properties;
    if(previous_highlight.m_order_visibility !=
        current_highlight.m_order_visibility) {
      auto log = QString();
      log += QString::number(++m_line_number) +
        QString(": Order highlight visibility: %1").
          arg(to_text(current_highlight.m_order_visibility));
      m_logs.append(log);
    }
    if(previous_highlight.m_order_highlights !=
        current_highlight.m_order_highlights) {
      auto log = QString();
      for(auto i = 0; i < std::ssize(current_highlight.m_order_highlights);
          ++i) {
        if(previous_highlight.m_order_highlights[i] !=
          current_highlight.m_order_highlights[i]) {
          auto& highlight =
            current_highlight.m_order_highlights[i];
          auto state =
            static_cast<BookViewHighlightProperties::OrderHighlightState>(i);
          log += QString::number(++m_line_number) +
            QString(": %1 order highlight: [%2, %3]").
              arg(to_text(state)).
              arg(highlight.m_background_color.name(QColor::HexArgb)).
              arg(highlight.m_text_color.name(QColor::HexArgb));
        }
      }
      m_logs.append(log);
    }
    if(previous_highlight.m_market_highlights.size() !=
        current_highlight.m_market_highlights.size()) {
      auto log = QString();
      log += QString::number(++m_line_number) + QString(": Market highlights{");
      for(auto i = 0; i < std::ssize(current_highlight.m_market_highlights);
          ++i) {
        auto& highlight = current_highlight.m_market_highlights[i];
        log += QString(" [%1, %2, [%3, %4]]").
          arg(GetDefaultMarketDatabase().FromCode(
            highlight.m_market).m_displayName.c_str()).
          arg(to_text(highlight.m_level)).
          arg(highlight.m_color.m_background_color.name(QColor::HexArgb)).
          arg(highlight.m_color.m_text_color.name(QColor::HexArgb));
      }
      log += "}";
      m_logs.append(log);
    }
    m_previous_properties = properties;
  }

  void on_default_quantity_update(const Quantity& quantity) {
    if(m_previous_interactions.get_default_quantity()->get() != quantity) {
      auto log = QString();
      log += QString::number(++m_line_number) +
        QString(": Default quantity: %1").arg(to_text(quantity));
      m_logs.append(log);
      m_previous_interactions.get_default_quantity()->set(quantity);
    }
  }

  void on_quantity_increment_update(Qt::KeyboardModifier modifier,
      const Quantity& quantity) {
    if(m_previous_interactions.get_quantity_increment(modifier)->get() !=
        quantity) {
      auto log = QString();
      log += QString::number(++m_line_number) +
        QString(": %1 Quantity increment: %2").
          arg(::to_text(modifier)).arg(to_text(quantity));
      m_logs.append(log);
      m_previous_interactions.get_quantity_increment(modifier)->set(quantity);
    }
  }

  void on_price_increment_update(Qt::KeyboardModifier modifier,
      const Money& money) {
    if(m_previous_interactions.get_price_increment(modifier)->get() != money) {
      auto log = QString();
      log += QString::number(++m_line_number) +
        QString(": %1 Price increment: %2").
          arg(::to_text(modifier)).arg(to_text(money));
      m_logs.append(log);
      m_previous_interactions.get_price_increment(modifier)->set(money);
    }
  }

  void on_cancel_on_fill(bool is_cancel_on_fill) {
    if(m_previous_interactions.is_cancel_on_fill()->get() !=
        is_cancel_on_fill) {
      auto log = QString();
      log += QString::number(++m_line_number) +
        QString(": Cancel on fill: %1").arg(is_cancel_on_fill);
      m_logs.append(log);
      m_previous_interactions.is_cancel_on_fill()->set(is_cancel_on_fill);
    }
  }

  void on_security_update(const Security& security) {
    auto interactions = m_key_bindings->get_interactions_key_bindings(security);
    copy_interactions(*interactions, m_previous_interactions);
    m_interaction_connections[0] =
      interactions->get_default_quantity()->connect_update_signal(
        std::bind_front(&PropertiesTester::on_default_quantity_update, this));
    auto count = 1;
    for(auto i :
        std::views::iota(0, InteractionsKeyBindingsModel::MODIFIER_COUNT)) {
      auto modifier = to_modifier(i);
      m_interaction_connections[count++] =
        interactions->get_quantity_increment(modifier)->connect_update_signal(
          std::bind_front(&PropertiesTester::on_quantity_increment_update, this,
            modifier));
      m_interaction_connections[count++] =
        interactions->get_price_increment(modifier)->connect_update_signal(
          std::bind_front(&PropertiesTester::on_price_increment_update, this,
            modifier));
    }
    m_interaction_connections[count++] =
      interactions->is_cancel_on_fill()->connect_update_signal(
        std::bind_front(&PropertiesTester::on_cancel_on_fill, this));
  }
};

struct BookViewTester : QWidget {
  BookViewTester(std::shared_ptr<SecurityTechnicalsModel> technicals,
      std::shared_ptr<ValueModel<BboQuote>> bbo_quote,
      std::shared_ptr<QuantityModel> default_bid_quantity,
      std::shared_ptr<QuantityModel> default_ask_quantity,
      std::shared_ptr<OptionalIntegerModel> font_size,
      std::shared_ptr<LocalSecurityModel> security,
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
    properties_layout->addRow(tr("Security:"),
      new SecurityBox(populate_security_query_model(), std::move(security)));
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
  auto security =
    std::make_shared<LocalSecurityModel>(ParseSecurity("MRU.TSX"));
  auto properties = std::make_shared<LocalBookViewPropertiesModel>(
    BookViewProperties(BookViewLevelProperties::get_default(),
      BookViewHighlightProperties::get_default()));
  auto key_bindings =
    std::make_shared<KeyBindingsModel>(GetDefaultMarketDatabase());
  auto key_bindings_window = KeyBindingsWindow(key_bindings,
    populate_security_query_model(), GetDefaultCountryDatabase(),
    GetDefaultMarketDatabase(), GetDefaultDestinationDatabase(),
    get_default_additional_tag_database());
  key_bindings_window.show();
  auto tester = BookViewTester(technicals, bbo_quote,
    default_bid_quantity, default_ask_quantity, font_size, security);
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
  auto properties_tester = PropertiesTester(properties, key_bindings, security);
  application.exec();
}
