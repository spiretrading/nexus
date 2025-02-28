#include <QApplication>
#include <array>
#include <QTextEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QRandomGenerator>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/BookViewUiTester/DemoBookViewModel.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/LocalTechnicalsModel.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/SideBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::Styles;

using OrderSatusBox = EnumBox<OrderStatus>;

std::time_t to_time_t_milliseconds(ptime pt) {
  return (pt - ptime(gregorian::date(1970, 1, 1))).total_milliseconds();
}

const QString& to_text(CancelKeyBindingsModel::Operation operation) {
  if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT) {
    static const auto value = QObject::tr("Most Recent");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK) {
    static const auto value = QObject::tr("Most Recent Ask");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID) {
    static const auto value = QObject::tr("Most Recent Bid");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::OLDEST) {
    static const auto value = QObject::tr("Oldest");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::OLDEST_ASK) {
    static const auto value = QObject::tr("Oldest Ask");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::OLDEST_BID) {
    static const auto value = QObject::tr("Oldest bid");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::ALL) {
    static const auto value = QObject::tr("All");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::ALL_ASKS) {
    static const auto value = QObject::tr("All Ask");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::ALL_BIDS) {
    static const auto value = QObject::tr("All bid");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK) {
    static const auto value = QObject::tr("Closest Ask");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::CLOSEST_BID) {
    static const auto value = QObject::tr("Closest bid");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::FURTHEST_ASK) {
    static const auto value = QObject::tr("Furthest Ask");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::FURTHEST_BID) {
    static const auto value = QObject::tr("Furthest bid");
    return value;
  }
  static const auto none = QString();
  return none;
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

BookQuote make_random_market_quote(Side side) {
  auto random_generator =
    QRandomGenerator(to_time_t_milliseconds(microsec_clock::universal_time()));
  auto& markets = GetDefaultMarketDatabase().GetEntries();
  auto market_index = random_generator.bounded(
    static_cast<int>(markets.size()));
  auto market_code = markets[market_index].m_code;
  return BookQuote(to_text(MarketToken(market_code)).toStdString(), false,
    market_code, Quote{Truncate(Money(random_generator.bounded(200.0)), 2),
      random_generator.bounded(1000), side}, second_clock::local_time());
}

OrderStatus make_order_status(int index) {
  static auto statuses = std::array<OrderStatus, 3>{
    OrderStatus::FILLED, OrderStatus::CANCELED, OrderStatus::REJECTED};
  return statuses[index % statuses.size()];
}

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
  DemoBookViewModel m_model;
  LocalTechnicalsModel m_technicals_model;
  std::shared_ptr<OptionalIntegerModel> m_update_period;
  KeyBindingsWindow* m_key_bindings_window;
  Button* m_submit_order_button;
  QTextEdit* m_logs;
  QTimer m_quote_timer;
  QTimer m_order_timer;
  int m_update_count;
  int m_line_number;

  BookViewTester(std::shared_ptr<SecurityTechnicalsValueModel> technicals,
    std::shared_ptr<BookViewModel> model,
    KeyBindingsWindow& key_bindings_window, QWidget* parent = nullptr)
      : QWidget(parent),
        m_model(std::move(model)),
        m_technicals_model(Security()),
        m_key_bindings_window(&key_bindings_window),
        m_update_period(std::make_shared<LocalOptionalIntegerModel>(1000)),
        m_quote_timer(this),
        m_order_timer(this),
        m_update_count(0),
        m_line_number(0) {
    auto left_layout = new QVBoxLayout();
    auto book_quote_group_box = new QGroupBox(tr("Market Quote"));
    book_quote_group_box->setSizePolicy(QSizePolicy::Preferred,
      QSizePolicy::Fixed);
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
    auto quote_update_period_box = new IntegerBox(m_update_period);
    book_quote_period_layout->addRow(tr("Update Period (ms):"),
      quote_update_period_box);
    book_quote_layout->addLayout(book_quote_period_layout);
    left_layout->addWidget(book_quote_group_box);
    auto order_status_group_box = new QGroupBox(tr("Order"));
    order_status_group_box->setSizePolicy(QSizePolicy::Preferred,
      QSizePolicy::Fixed);
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
    m_submit_order_button = make_label_button(tr("Submit"));
    m_submit_order_button->connect_click_signal(
      std::bind_front(&BookViewTester::on_order_submit_click, this,
        order_status_destination_box, order_status_price_box,
        order_quantity_box, order_status_box, order_status_side_box,
        continuous_update_box, status_update_count_box,
        status_update_period_box));
    order_status_layout->addWidget(m_submit_order_button, 0,
      Qt::AlignRight);
    left_layout->addWidget(order_status_group_box);
    auto preview_order_group_box = new QGroupBox(tr("Preview Order"));
    preview_order_group_box->setSizePolicy(QSizePolicy::Preferred,
      QSizePolicy::Fixed);
    auto preview_order_layout = new QVBoxLayout(preview_order_group_box);
    auto preview_order_fields_layout = new QFormLayout();
    auto preview_order_destination_box = new TextBox();
    preview_order_fields_layout->addRow(tr("Destination:"),
      preview_order_destination_box);
    auto preview_order = m_model.get_model()->get_preview_order();
    auto preview_order_price = std::make_shared<LocalOptionalMoneyModel>(
      Money(200));
    preview_order_price->connect_update_signal([=] (auto& price) {
      auto order = preview_order->get();
      if(order && price) {
        order->m_price = *price;
        preview_order->set(order);
      }
    });
    auto preview_order_price_box = new MoneyBox(preview_order_price);
    preview_order_fields_layout->addRow(tr("Price:"), preview_order_price_box);
    auto preview_order_quantity = std::make_shared<LocalOptionalQuantityModel>(
      Quantity(10));
    preview_order_quantity->connect_update_signal([=] (auto& quantity) {
      auto order = preview_order->get();
      if(order && quantity) {
        order->m_quantity = *quantity;
        preview_order->set(order);
      }
    });
    auto preview_order_quantity_box = new QuantityBox(preview_order_quantity);
    preview_order_fields_layout->addRow(tr("Quantity:"),
      preview_order_quantity_box);
    auto preview_order_side_box = make_side_box();
    preview_order_fields_layout->addRow(tr("Side:"), preview_order_side_box);
    preview_order_layout->addLayout(preview_order_fields_layout);
    auto buttons_layout = new QHBoxLayout();
    auto create_preview_order_button = make_label_button(tr("Create"));
    auto submit_preview_order_button = make_label_button(tr("Submit"));
    submit_preview_order_button->setEnabled(false);
    create_preview_order_button->connect_click_signal([=] {
      create_preview_order_button->setEnabled(false);
      submit_preview_order_button->setEnabled(true);
      m_model.get_model()->get_preview_order()->set(OrderFields::MakeLimitOrder(
        Security(), preview_order_side_box->get_current()->get(),
        preview_order_destination_box->get_current()->get().toStdString(),
        *preview_order_quantity_box->get_current()->get(),
        *preview_order_price_box->get_current()->get()));
    });
    submit_preview_order_button->connect_click_signal([=] {
      create_preview_order_button->setEnabled(true);
      submit_preview_order_button->setEnabled(false);
      m_model.get_model()->get_preview_order()->set(none);
    });
    buttons_layout->addWidget(create_preview_order_button);
    buttons_layout->addWidget(submit_preview_order_button);
    preview_order_layout->addLayout(buttons_layout);
    left_layout->addWidget(preview_order_group_box);
    auto key_bindings_button = make_label_button(tr("Key Bindings"));
    key_bindings_button->connect_click_signal([=] {
      m_key_bindings_window->show();
    });
    left_layout->addWidget(key_bindings_button);
    left_layout->addStretch(1);
    auto right_layout = new QVBoxLayout();
    m_logs = new QTextEdit();
    right_layout->addWidget(m_logs);
    auto layout = new QHBoxLayout(this);
    layout->addLayout(left_layout);
    layout->addLayout(right_layout);
    setFixedWidth(scale_width(550));
    connect(&m_quote_timer, &QTimer::timeout,
      std::bind_front(&BookViewTester::on_quote_timeout, this));
    m_update_period->connect_update_signal([=] (auto& period) {
      if(period) {
        m_quote_timer.setInterval(*period);
      }
    });
    m_technicals_model.connect_high_slot([=] (Nexus::Money value) {
      auto t = technicals->get();
      t.m_high = value;
      technicals->set(t);
    });
    m_technicals_model.connect_low_slot([=] (Nexus::Money value) {
      auto t = technicals->get();
      t.m_low = value;
      technicals->set(t);
    });
    m_technicals_model.connect_open_slot([=] (Nexus::Money value) {
      auto t = technicals->get();
      t.m_open = value;
      technicals->set(t);
    });
    m_technicals_model.connect_close_slot([=] (Nexus::Money value) {
      auto t = technicals->get();
      t.m_close = value;
      technicals->set(t);
    });
    m_technicals_model.connect_volume_slot([=] (Nexus::Quantity value) {
      auto t = technicals->get();
      t.m_volume = value;
      technicals->set(t);
    });
  }

  bool eventFilter(QObject* object, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      QApplication::quit();
    }
    return QWidget::eventFilter(object, event);
  }

  void start_populate() {
    m_quote_timer.start(*m_update_period->get());
  }

  void quit_order_timer() {
    m_order_timer.stop();
    m_order_timer.disconnect();
    m_submit_order_button->setEnabled(true);
  }

  void on_quote_timeout() {
    auto bid_quote = make_random_market_quote(Side::BID);
    m_model.submit_book_quote(bid_quote);
    auto ask_quote = make_random_market_quote(Side::ASK);
    m_model.submit_book_quote(ask_quote);
    m_technicals_model.update(TimeAndSale(bid_quote.m_timestamp,
      bid_quote.m_quote.m_price, bid_quote.m_quote.m_size,
      TimeAndSale::Condition(), bid_quote.m_market.GetData(), bid_quote.m_mpid,
      ""));
    m_technicals_model.update(TimeAndSale(ask_quote.m_timestamp,
      ask_quote.m_quote.m_price, ask_quote.m_quote.m_size,
      TimeAndSale::Condition(), ask_quote.m_market.GetData(), "",
      ask_quote.m_mpid));
  }

  void on_order_timeout(const DemoBookViewModel::OrderInfo& order,
      int update_count) {
    auto updated_order = order;
    updated_order.m_status = make_order_status(m_update_count);
    m_model.update_order_status(updated_order);
    ++m_update_count;
    if(m_update_count >= update_count) {
      quit_order_timer();
    }
  }

  void on_book_quote_submit_click(MarketBox* market_box,
      MoneyBox* price_box, QuantityBox* quantity_box, SideBox* side_box) {
    auto market_code = market_box->get_current()->get();
    auto quote = BookQuote(to_text(MarketToken(market_code)).toStdString(),
        false, market_code, Quote{*price_box->get_current()->get(),
        *quantity_box->get_current()->get(), side_box->get_current()->get()},
        second_clock::local_time());
    m_model.submit_book_quote(quote);
  }

  void on_order_submit_click(TextBox* destination_box, MoneyBox* price_box,
      QuantityBox* quantity_box, OrderSatusBox* status_box, SideBox* side_box,
      CheckBox* continuous_update_box, IntegerBox* update_count_box,
      IntegerBox* update_period_box) {
    auto order = DemoBookViewModel::OrderInfo{
      OrderFields::MakeLimitOrder(Security(), side_box->get_current()->get(),
        destination_box->get_current()->get().toStdString(),
        *quantity_box->get_current()->get(),
        *price_box->get_current()->get()), status_box->get_current()->get()};
    if(!continuous_update_box->get_current()->get()) {
      m_model.submit_order(order);
    } else {
      auto update_count = *update_count_box->get_current()->get();
      while(update_count--) {
        m_model.submit_order(order);
      }
      m_submit_order_button->setEnabled(false);
      m_order_timer.setInterval(*update_period_box->get_current()->get());
      connect(&m_order_timer, &QTimer::timeout,
        std::bind_front(&BookViewTester::on_order_timeout, this, order,
          *update_count_box->get_current()->get()));
      m_update_count = 0;
      m_order_timer.start();
    }
  }

  void on_cancel_order(CancelKeyBindingsModel::Operation operation,
      const Security& security,
      const optional<std::tuple<Destination, Money>>& order_key) {
    m_model.cancel_orders(operation, order_key);
    auto log = QString("%1: Operation:[%2] Security:[%3]").
      arg(++m_line_number).
      arg(to_text(operation)).
      arg(to_text(security));
    if(order_key) {
      log += QString(" Order Destination:[%1] Order Price:[%2]").
        arg(QString::fromStdString(std::get<0>(*order_key))).
        arg(to_text(std::get<1>(*order_key)));
    }
    m_logs->append(log);
  }
};

std::shared_ptr<BookViewModel> model_builder(
    std::shared_ptr<BookViewModel> model, BookViewTester* tester,
    const Security&) {
  model->get_preview_order()->set(none);
  clear(*model->get_bids());
  clear(*model->get_asks());
  clear(*model->get_bid_orders());
  clear(*model->get_ask_orders());
  model->get_bbo_quote()->set({});
  tester->start_populate();
  return model;
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("BookView Ui Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto technicals = std::make_shared<LocalSecurityTechnicalsValueModel>(
    SecurityTechnicals(1100, Money(144.4), Money(142.11), Money(144.25),
      Money(144.99)));
  auto bbo_quote = std::make_shared<LocalValueModel<BboQuote>>(
    BboQuote(Quote(Money(143.53), 51, Side::BID),
      Quote(Money(143.54), 39, Side::ASK), second_clock::universal_time()));
  auto default_bid_quantity = std::make_shared<LocalQuantityModel>(100);
  auto default_ask_quantity = std::make_shared<LocalQuantityModel>(100);
  auto properties = std::make_shared<LocalBookViewPropertiesModel>(
    BookViewProperties(BookViewLevelProperties::get_default(),
      BookViewHighlightProperties::get_default()));
  auto key_bindings =
    std::make_shared<KeyBindingsModel>(GetDefaultMarketDatabase());
  auto key_bindings_window = KeyBindingsWindow(key_bindings,
    populate_security_query_model(), GetDefaultCountryDatabase(),
    GetDefaultMarketDatabase(), GetDefaultDestinationDatabase(),
    get_default_additional_tag_database());
  auto book_views = std::make_shared<BookViewModel>();
  auto tester = BookViewTester(book_views->get_technicals(), book_views,
    key_bindings_window);
  auto markets = GetDefaultMarketDatabase();
  auto window = BookViewWindow(populate_security_query_model(), key_bindings,
    markets, std::make_shared<BookViewPropertiesWindowFactory>(),
    std::bind_front(&model_builder, book_views, &tester));
  window.connect_cancel_order_signal(
    std::bind_front(&BookViewTester::on_cancel_order, &tester));
  window.installEventFilter(&tester);
  window.show();
  tester.show();
  tester.move(
    window.pos().x() + window.frameGeometry().width() + scale_width(100),
    window.pos().y() - 200);
  application.exec();
}
