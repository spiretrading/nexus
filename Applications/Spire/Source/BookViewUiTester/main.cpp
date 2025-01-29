#include <QApplication>
#include <array>
#include <ranges>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGroupBox>
#include <QRandomGenerator>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Spire/BookView/BboBox.hpp"
#include "Spire/BookView/BookViewPropertiesWindow.hpp"
#include "Spire/BookView/BookViewTableView.hpp"
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
#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::Styles;

std::time_t to_time_t_milliseconds(ptime pt) {
  return (pt - ptime(gregorian::date(1970, 1, 1))).total_milliseconds();
}

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

BookQuote make_random_market_quote() {
  auto random_generator =
    QRandomGenerator(to_time_t_milliseconds(microsec_clock::universal_time()));
  auto& markets = GetDefaultMarketDatabase().GetEntries();
  auto market_index = random_generator.bounded(markets.size());
  auto market_code = markets[market_index].m_code;
  return BookQuote(to_text(MarketToken(market_code)).toStdString(), false,
    market_code, Quote{Truncate(Money(random_generator.bounded(200.0)), 2),
      random_generator.bounded(1000), Side::BID}, second_clock::local_time());
}

OrderStatus make_order_status(int index) {
  static auto statuses = std::array<OrderStatus, 3>{
    OrderStatus::FILLED, OrderStatus::CANCELED, OrderStatus::REJECTED};
  return statuses[index % statuses.size()];
}

using OrderSatusBox = EnumBox<OrderStatus>;

OrderSatusBox* make_order_status_box(QWidget* parent = nullptr) {
  auto settings = OrderSatusBox::Settings();
  auto cases = std::make_shared<ArrayListModel<OrderStatus>>();
  cases->push(OrderStatus::NEW);
  cases->push(OrderStatus::FILLED);
  cases->push(OrderStatus::CANCELED);
  cases->push(OrderStatus::REJECTED);
  settings.m_cases = std::move(cases);
  return new OrderSatusBox(std::move(settings), parent);
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

struct BookViewTester : QWidget {
  std::shared_ptr<BookViewModel> m_model;
  QTimer m_quote_timer;
  QTimer m_order_timer;
  int m_update_count;

  BookViewTester(std::shared_ptr<SecurityTechnicalsModel> technicals,
      std::shared_ptr<ValueModel<BboQuote>> bbo_quote,
      std::shared_ptr<QuantityModel> default_bid_quantity,
      std::shared_ptr<QuantityModel> default_ask_quantity,
      std::shared_ptr<OptionalIntegerModel> font_size,
      std::shared_ptr<LocalSecurityModel> security,
      std::shared_ptr<BookViewModel> model,
      QWidget* parent = nullptr)
      : QWidget(parent),
        m_model(std::move(model)),
        m_quote_timer(this),
        m_order_timer(this),
        m_update_count(0) {
    auto left_layout = new QVBoxLayout();
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
    left_layout->addWidget(technicals_group_box);
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
    left_layout->addWidget(bbo_quote_group_box);
    auto properties_group_box = new QGroupBox(tr("Properties"));
    auto properties_layout = new QFormLayout(properties_group_box);
    properties_layout->addRow(tr("Font Size:"),
      new IntegerBox(std::move(font_size)));
    properties_layout->addRow(tr("Security:"),
      new SecurityBox(populate_security_query_model(), security));
    left_layout->addWidget(properties_group_box);
    auto right_layout = new QVBoxLayout();
    auto book_quote_group_box = new QGroupBox(tr("Market Quote"));
    auto book_quote_layout = new QVBoxLayout(book_quote_group_box);
    auto book_quote_fields_layout = new QFormLayout();
    auto book_quote_market_box =
      make_market_box(std::make_shared<LocalMarketModel>("ARCX"),
        GetDefaultMarketDatabase());
    book_quote_fields_layout->addRow(tr("MPID:"), book_quote_market_box);
    auto book_quote_price_box =
      new MoneyBox(std::make_shared<LocalOptionalMoneyModel>(Money(200)));
    book_quote_fields_layout->addRow(tr("Price:"), book_quote_price_box);
    auto book_quote_quantity_box = new QuantityBox(
      std::make_shared<LocalOptionalQuantityModel>(Quantity(10)));
    book_quote_fields_layout->addRow(tr("Quantity:"), book_quote_quantity_box);
    auto book_quote_side_box = make_side_box();
    book_quote_fields_layout->addRow(tr("Side:"), book_quote_side_box);
    book_quote_layout->addLayout(book_quote_fields_layout);
    auto submit_quote_button = make_label_button(tr("Submit"));
    submit_quote_button->connect_click_signal(
      std::bind_front(&BookViewTester::on_book_quote_submit_click, this,
        book_quote_market_box, book_quote_price_box,
        book_quote_quantity_box, book_quote_side_box));
    book_quote_layout->addWidget(submit_quote_button, 0, Qt::AlignRight);
    auto book_quote_period_layout = new QFormLayout();
    auto quote_update_period =
      std::make_shared<LocalOptionalIntegerModel>(1000);
    quote_update_period->connect_update_signal([=] (auto& period) {
      if(period) {
        m_quote_timer.setInterval(*period);
      }
    });
    auto quote_update_period_box = new IntegerBox(quote_update_period);
    book_quote_period_layout->addRow(tr("Update Period (ms):"),
      quote_update_period_box);
    book_quote_layout->addLayout(book_quote_period_layout);
    right_layout->addWidget(book_quote_group_box);
    auto order_status_group_box = new QGroupBox(tr("Order"));
    auto order_status_layout = new QVBoxLayout(order_status_group_box);
    auto order_status_fields_layout = new QFormLayout();
    auto order_status_destination_box = new TextBox();
    order_status_fields_layout->addRow(tr("Destination:"),
      order_status_destination_box);
    auto order_status_price_box =
      new MoneyBox(std::make_shared<LocalOptionalMoneyModel>(Money(200)));
    order_status_fields_layout->addRow(tr("Price:"), order_status_price_box);
    auto order_status_side_box = make_side_box();
    auto order_quantity_box = new QuantityBox(
      std::make_shared<LocalOptionalQuantityModel>(Quantity(10)));
    order_status_fields_layout->addRow(tr("Quantity:"), order_quantity_box);
    order_status_fields_layout->addRow(tr("Side:"), order_status_side_box);
    auto order_status_box = make_order_status_box();
    order_status_fields_layout->addRow(tr("Order Status:"), order_status_box);
    auto continuous_update_box = new CheckBox();
    order_status_fields_layout->addRow(tr("Continuous Update:"),
      continuous_update_box);
    auto status_update_count_box = new IntegerBox();
    status_update_count_box->get_current()->set(3);
    order_status_fields_layout->addRow(tr("Update count:"),
      status_update_count_box);
    auto status_update_period_box = new IntegerBox();
    status_update_period_box->get_current()->set(10);
    order_status_fields_layout->addRow(tr("Update Period (ms):"),
      status_update_period_box);
    order_status_layout->addLayout(order_status_fields_layout);
    auto submit_order_button = make_label_button(tr("Submit"));
    submit_order_button->connect_click_signal(
      std::bind_front(&BookViewTester::on_order_submit_click, this,
        order_status_destination_box, order_status_price_box,
        order_quantity_box, order_status_box, order_status_side_box,
        continuous_update_box, status_update_count_box,
        status_update_period_box, submit_order_button));
    order_status_layout->addWidget(submit_order_button, 0,
      Qt::AlignRight);
    right_layout->addWidget(order_status_group_box);
    right_layout->addStretch(1);
    auto layout = new QHBoxLayout(this);
    layout->addLayout(left_layout);
    layout->addLayout(right_layout);
    setFixedWidth(scale_width(550));
    connect(&m_quote_timer, &QTimer::timeout,
      std::bind_front(&BookViewTester::on_quote_timeout, this));
    m_quote_timer.start(*quote_update_period->get());
  }

  bool eventFilter(QObject* object, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      QApplication::quit();
    }
    return QWidget::eventFilter(object, event);
  }

  int find_book_quote(const ListModel<BookQuote>& quotes,
      const BookQuote& quote) {
    auto i = std::find_if(quotes.begin(), quotes.end(),
      [&] (const BookQuote& value) {
        return value.m_mpid == quote.m_mpid &&
          value.m_quote.m_price == quote.m_quote.m_price;
      });
    if(i == quotes.end()) {
      return -1;
    }
    return std::distance(quotes.begin(), i);
  }

  int find_order(const ListModel<BookViewModel::UserOrder>& orders,
      const BookViewModel::UserOrder& order) {
    auto i = std::find_if(orders.begin(), orders.end(),
      [&] (const BookViewModel::UserOrder& value) {
        return value.m_destination == order.m_destination &&
          value.m_price == order.m_price;
      });
    if(i == orders.end()) {
      return -1;
    }
    return std::distance(orders.begin(), i);
  }

  void update_order(int quote_index, int order_index, Quantity remaining_size,
      BookViewModel::UserOrder& order, OrderStatus status) {
    if(quote_index < 0 || order_index < 0) {
      return;
    }
    remaining_size = std::max(Quantity(0), remaining_size);
    auto book_quote = m_model->get_bids()->get(quote_index);
    book_quote.m_quote.m_size = remaining_size;
    m_model->get_bids()->set(quote_index, book_quote);
    order.m_status = status;
    m_model->get_bid_orders()->set(order_index, order);
    order.m_status = OrderStatus::NONE;
    m_model->get_bid_orders()->set(order_index, order);
    if(remaining_size <= 0) {
      m_model->get_bid_orders()->remove(order_index);
    }
  }

  void on_quote_timeout() {
    m_model->get_bids()->push(make_random_market_quote());
  }

  void on_book_quote_submit_click(MarketBox* market_box,
      MoneyBox* price_box, QuantityBox* quantity_box, SideBox* side_box) {
    auto market_code = market_box->get_current()->get();
    auto quote = BookQuote(to_text(MarketToken(market_code)).toStdString(),
        false, market_code, Quote{*price_box->get_current()->get(),
        *quantity_box->get_current()->get(), side_box->get_current()->get()},
        second_clock::local_time());
    if(side_box->get_current()->get() == Side::BID) {
      auto i = find_book_quote(*m_model->get_bids(), quote);
      if(i >= 0) {
        if(*quantity_box->get_current()->get() == 0) {
          m_model->get_bids()->remove(i);
        } else {
          m_model->get_bids()->set(i, quote);
        }
      } else if(*quantity_box->get_current()->get() != 0) {
        m_model->get_bids()->push(quote);
      }
    }
  }

  void on_order_submit_click(TextBox* destination_box, MoneyBox* price_box,
      QuantityBox* quantity_box, OrderSatusBox* status_box, SideBox* side_box,
      CheckBox* continuous_update_box, IntegerBox* update_count_box,
      IntegerBox* update_period_box, Button* submit_order_button) {
    auto user_order = BookViewModel::UserOrder{
      destination_box->get_current()->get().toStdString(),
      *price_box->get_current()->get(), status_box->get_current()->get()};
    if(side_box->get_current()->get() == Side::BID) {
      auto mpid = "@" + destination_box->get_current()->get();
      auto quote = BookQuote(mpid.toStdString(), false, "",
        Quote{*price_box->get_current()->get(),
        *quantity_box->get_current()->get(), side_box->get_current()->get()},
        second_clock::local_time());
      auto order_index = find_order(*m_model->get_bid_orders(), user_order);
      if(order_index >= 0) {
        auto quote_index = find_book_quote(*m_model->get_bids(), quote);
        if(quote_index < 0) {
          return;
        }
        auto quantity = m_model->get_bids()->get(quote_index).m_quote.m_size;
        if(!continuous_update_box->get_current()->get()) {
          if(user_order.m_status == OrderStatus::NEW) {
            auto remaining_quantity =
              quantity + *quantity_box->get_current()->get();
            auto book_quote = m_model->get_bids()->get(quote_index);
            book_quote.m_quote.m_size = remaining_quantity;
            m_model->get_bids()->set(quote_index, book_quote);
          } else {
            update_order(quote_index, order_index,
              quantity - *quantity_box->get_current()->get(),
              user_order, user_order.m_status);
          }
        } else {
          auto quit_timer = [=] {
            m_order_timer.stop();
            m_order_timer.disconnect();
            submit_order_button->setEnabled(true);
          };
          submit_order_button->setEnabled(false);
          m_order_timer.setInterval(*update_period_box->get_current()->get());
          connect(&m_order_timer, &QTimer::timeout, [=] () mutable {
            auto quote_index = find_book_quote(*m_model->get_bids(), quote);
            if(quote_index < 0) {
              quit_timer();
              return;
            }
            auto size = m_model->get_bids()->get(quote_index).m_quote.m_size;
            update_order(quote_index,
              find_order(*m_model->get_bid_orders(), user_order),
              size - *quantity_box->get_current()->get(),
              user_order, make_order_status(m_update_count));
            ++m_update_count;
            if(m_update_count >= update_count_box->get_current()->get()) {
              quit_timer();
            }
          });
          m_update_count = 0;
          m_order_timer.start();
        }
      } else if(user_order.m_status == OrderStatus::NEW &&
          !continuous_update_box->get_current()->get()) {
        m_model->get_bids()->push(quote);
        m_model->get_bid_orders()->push(user_order);
      }
    }
  }
};

struct MarketDepthContainer : QWidget {
  MarketDepthContainer(std::shared_ptr<BookViewModel> model,
      std::shared_ptr<ValueModel<BboQuote>> bbo_quote,
      std::shared_ptr<BookViewPropertiesModel> properties,
      std::shared_ptr<OptionalIntegerModel> font_size,
      QWidget* parent = nullptr)
      : QWidget(parent) {
    auto layout = make_hbox_layout(this);
    layout->setSpacing(scale_width(2));
    auto bid_panel = new QWidget();
    bid_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto bid_layout = make_vbox_layout(bid_panel);
    auto bid_bbo =
      new BboBox(make_field_value_model(bbo_quote, &BboQuote::m_bid));
    bid_bbo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bid_layout->addWidget(bid_bbo);
    auto bid_table_view = make_book_view_table_view(model, properties,
      Side::BID, GetDefaultMarketDatabase());
    bid_table_view->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    bid_layout->addWidget(bid_table_view);
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
  auto book_views = std::make_shared<BookViewModel>();
  auto tester = BookViewTester(technicals, bbo_quote, default_bid_quantity,
    default_ask_quantity, font_size, security, book_views);
  auto widget = QWidget();
  auto layout = make_vbox_layout(&widget);
  auto panel = new TechnicalsPanel(technicals, default_bid_quantity,
    default_ask_quantity);
  panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(panel);
  layout->addWidget(
    new MarketDepthContainer(book_views, bbo_quote, properties, font_size));
  widget.installEventFilter(&tester);
  widget.show();
  widget.resize(scale(206, 417));
  tester.show();
  tester.move(
    tester.pos().x() + widget.frameGeometry().width() + scale_width(100),
    widget.pos().y() - 200);
  BookViewPropertiesWindow properties_window(properties,
    key_bindings, security, GetDefaultMarketDatabase());
  properties_window.show();
  properties_window.move(
    tester.pos().x() + widget.frameGeometry().width() + scale_width(300),
    widget.pos().y());
  application.exec();
}
