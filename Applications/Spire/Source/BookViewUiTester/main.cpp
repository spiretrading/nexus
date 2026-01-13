#include <QApplication>
#include <array>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <QTextEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QRandomGenerator>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/BookView/AggregateBookViewModel.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/BookViewUiTester/DemoBookViewModel.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/LocalTechnicalsModel.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/VenueBox.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace Spire::Styles;

using OrderStatusBox = EnumBox<OrderStatus>;

const auto ORDER_COLUMN_COUNT = 4;

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
    static const auto value = QObject::tr("All Asks");
    return value;
  } else if(operation == CancelKeyBindingsModel::Operation::ALL_BIDS) {
    static const auto value = QObject::tr("All bids");
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
  auto add_security = [&] (const Security& security, const std::string& name) {
    if(security) {
      security_infos.emplace_back(security, name, "", 0);
    }
  };
  add_security(parse_security("MRU.TSX"), "Metro Inc.");
  add_security(parse_security("MG.TSX"), "Magna International Inc.");
  add_security(parse_security("MGA.TSX"), "Mega Uranium Ltd.");
  add_security(parse_security("MGAB.TSX"),
    "Mackenzie Global Fixed Income Alloc ETF");
  add_security(parse_security("MON.NYSE"), "Monsanto Co.");
  add_security(parse_security("MFC.TSX"), "Manulife Financial Corporation");
  add_security(parse_security("MX.TSX"), "Methanex Corporation");
  auto model = std::make_shared<LocalQueryModel<SecurityInfo>>();
  for(auto& security_info : security_infos) {
    model->add(to_text(security_info.m_security).toLower(), security_info);
    model->add(
      QString::fromStdString(security_info.m_name).toLower(), security_info);
  }
  return model;
}

BookQuote make_random_venue_quote(Side side) {
  auto random_generator =
    QRandomGenerator(to_time_t_milliseconds(microsec_clock::universal_time()));
  auto venues = DEFAULT_VENUES.get_entries();
  auto venue_index = random_generator.bounded(
    static_cast<int>(venues.size()));
  auto venue_code = venues[venue_index].m_venue;
  return BookQuote(to_text(venue_code).toStdString(), true, venue_code,
    Quote(truncate_to(Money(random_generator.bounded(200.0)), Money::CENT),
      random_generator.bounded(1000), side), second_clock::local_time());
}

OrderStatusBox* make_order_status_box(QWidget* parent = nullptr) {
  auto settings = OrderStatusBox::Settings();
  auto cases = std::make_shared<ArrayListModel<OrderStatus>>();
  cases->push(OrderStatus::NEW);
  cases->push(OrderStatus::FILLED);
  cases->push(OrderStatus::CANCELED);
  cases->push(OrderStatus::REJECTED);
  cases->push(OrderStatus::PARTIALLY_FILLED);
  settings.m_cases = std::move(cases);
  return new OrderStatusBox(std::move(settings), parent);
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

auto make_table_header() {
  auto header = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
  auto item = TableHeaderItem::Model();
  item.m_name = "Destination";
  item.m_order = TableHeaderItem::Order::UNORDERED;
  header->push(item);
  item = TableHeaderItem::Model();
  item.m_order = TableHeaderItem::Order::UNORDERED;
  item.m_name = "Price";
  header->push(item);
  item = TableHeaderItem::Model();
  item.m_order = TableHeaderItem::Order::UNORDERED;
  item.m_name = "Quantity";
  header->push(item);
  item = TableHeaderItem::Model();
  item.m_order = TableHeaderItem::Order::UNORDERED;
  item.m_name = "Status";
  header->push(item);
  return header;
}

AnyRef extract_column(const BookViewModel::UserOrder& order, int index) {
  if(index == 0) {
    return order.m_destination;
  } else if(index == 1) {
    return order.m_price;
  } else if(index == 2) {
    return order.m_size;
  }
  return order.m_status;
}

auto get_orders(const BookViewModel& model, Side side) {
  if(side == Side::BID) {
    return model.get_bid_orders();
  }
  return model.get_ask_orders();
}

struct BookViewOrderTester : QWidget {
  std::shared_ptr<BookViewModel> m_model;
  TextBox* m_destination_box;
  MoneyBox* m_price_box;
  QuantityBox* m_quantity_box;
  SideBox* m_side_box;
  OrderStatusBox* m_status_box;
  TableView* m_bid_table_view;
  TableView* m_ask_table_view;

  BookViewOrderTester(std::shared_ptr<BookViewModel> model,
      QWidget* parent = nullptr)
      : QWidget(parent),
        m_model(std::move(model)) {
    auto order_group_box = new QGroupBox(tr("Order"));
    order_group_box->setFixedWidth(scale_width(200));
    auto order_group_layout = new QVBoxLayout(order_group_box);
    auto fields_layout = new QFormLayout();
    m_destination_box = new TextBox();
    fields_layout->addRow(tr("Destination:"), m_destination_box);
    m_price_box =
      new MoneyBox(std::make_shared<LocalOptionalMoneyModel>(Money(200)));
    fields_layout->addRow(tr("Price:"), m_price_box);
    m_quantity_box = new QuantityBox(
      std::make_shared<LocalOptionalQuantityModel>(Quantity(10)));
    fields_layout->addRow(tr("Quantity:"), m_quantity_box);
    m_side_box = make_side_box();
    fields_layout->addRow(tr("Side:"), m_side_box);
    m_status_box = make_order_status_box();
    fields_layout->addRow(tr("Order Status:"), m_status_box);
    order_group_layout->addLayout(fields_layout);
    auto left_layout = new QVBoxLayout();
    left_layout->addWidget(order_group_box);
    left_layout->addStretch(1);
    m_bid_table_view = TableViewBuilder(
      std::make_shared<ListToTableModel<BookViewModel::UserOrder>>(
        m_model->get_bid_orders(), ORDER_COLUMN_COUNT, &extract_column)).
      set_header(make_table_header()).make();
    m_bid_table_view->get_body().get_current()->connect_update_signal(
      std::bind_front(&BookViewOrderTester::on_current, this, Side::BID));
    auto bid_group_box = new QGroupBox(tr("Bid"));
    bid_group_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto bid_layout = new QVBoxLayout(bid_group_box);
    bid_layout->addWidget(m_bid_table_view);
    m_ask_table_view = TableViewBuilder(
      std::make_shared<ListToTableModel<BookViewModel::UserOrder>>(
        m_model->get_ask_orders(), ORDER_COLUMN_COUNT, &extract_column)).
      set_header(make_table_header()).make();
    m_ask_table_view->get_body().get_current()->connect_update_signal(
      std::bind_front(&BookViewOrderTester::on_current, this, Side::ASK));
    auto ask_group_box = new QGroupBox(tr("Ask"));
    ask_group_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto ask_layout = new QVBoxLayout(ask_group_box);
    ask_layout->addWidget(m_ask_table_view);
    auto right_layout = new QVBoxLayout();
    right_layout->addWidget(bid_group_box);
    right_layout->addWidget(ask_group_box);
    auto top_layout = new QHBoxLayout();
    top_layout->addLayout(left_layout);
    top_layout->addLayout(right_layout);
    auto add_order_button = make_label_button(tr("Add"));
    add_order_button->connect_click_signal(
      std::bind_front(&BookViewOrderTester::on_add_order_click, this));
    auto update_order_button = make_label_button(tr("Update"));
    update_order_button->connect_click_signal(
      std::bind_front(&BookViewOrderTester::on_update_order_click, this));
    auto delete_order_button = make_label_button(tr("Delete"));
    delete_order_button->connect_click_signal(
      std::bind_front(&BookViewOrderTester::on_delete_order_click, this));
    auto bottom_layout = new QHBoxLayout();
    bottom_layout->addWidget(add_order_button);
    bottom_layout->addWidget(update_order_button);
    bottom_layout->addWidget(delete_order_button);
    auto layout = new QVBoxLayout(this);
    layout->addLayout(top_layout);
    layout->addLayout(bottom_layout);
    resize(scale(420, 500));
  }

  void on_add_order_click() {
    auto orders = [&] {
      if(m_side_box->get_current()->get() == Side::BID) {
        return m_model->get_bid_orders();
      }
      return m_model->get_ask_orders();
    }();
    orders->push(BookViewModel::UserOrder(
      m_destination_box->get_current()->get().toStdString(),
      *m_price_box->get_current()->get(), *m_quantity_box->get_current()->get(),
      m_status_box->get_current()->get()));
  }

  void on_update_order_click() {
    auto [index, orders] = [&] {
      if(m_side_box->get_current()->get() == Side::BID) {
        return std::tuple(m_bid_table_view->get_current()->get(),
          m_model->get_bid_orders());
      }
      return std::tuple(m_ask_table_view->get_current()->get(),
        m_model->get_ask_orders());
    }();
    if(index) {
      auto user_order = orders->get(index->m_row);
      user_order.m_status = m_status_box->get_current()->get();
      if(m_status_box->get_current()->get() == OrderStatus::NEW) {
        user_order.m_size = *m_quantity_box->get_current()->get();
      } else if(m_status_box->get_current()->get() ==
          OrderStatus::PARTIALLY_FILLED) {
        user_order.m_size -= *m_quantity_box->get_current()->get();
      }
      orders->set(index->m_row, user_order);
    }
  }

  void on_delete_order_click() {
    auto [index, orders] = [&] {
      if(m_side_box->get_current()->get() == Side::BID) {
        return std::tuple(m_bid_table_view->get_current()->get(),
          m_model->get_bid_orders());
      }
      return std::tuple(m_ask_table_view->get_current()->get(),
        m_model->get_ask_orders());
    }();
    if(index) {
      orders->remove(index->m_row);
    }
  }

  void on_current(Side side, const optional<TableView::Index>& current) {
    if(!current) {
      return;
    }
    auto orders = get_orders(*m_model, side);
    auto order = orders->get(current->m_row);
    m_destination_box->get_current()->set(
      QString::fromStdString(order.m_destination));
    m_price_box->get_current()->set(order.m_price);
    m_quantity_box->get_current()->set(order.m_size);
    m_side_box->get_current()->set(side);
    m_status_box->get_current()->set(order.m_status);
  }
};

struct BookViewTester : QWidget {
  TestEnvironment m_environment;
  Clients m_clients;
  UserProfile m_user_profile;
  DemoBookViewModel m_model;
  LocalTechnicalsModel m_technicals_model;
  std::shared_ptr<OptionalIntegerModel> m_update_period;
  KeyBindingsWindow* m_key_bindings_window;
  QTextEdit* m_logs;
  QTimer m_quote_timer;
  int m_line_number;

  BookViewTester(std::shared_ptr<SecurityTechnicalsModel> technicals,
    std::shared_ptr<BookViewModel> model,
    KeyBindingsWindow& key_bindings_window, QWidget* parent = nullptr)
      : QWidget(parent),
        m_clients(std::in_place_type<TestClients>, Ref(m_environment)),
        m_user_profile("", false, false, {}, {},
          get_default_additional_tag_database(), {}, {}, m_clients),
        m_model(std::move(model)),
        m_technicals_model(Security()),
        m_key_bindings_window(&key_bindings_window),
        m_update_period(std::make_shared<LocalOptionalIntegerModel>(1000)),
BEAM_SUPPRESS_THIS_INITIALIZER()
        m_quote_timer(this),
BEAM_UNSUPPRESS_THIS_INITIALIZER()
        m_line_number(0) {
    auto& settings = m_user_profile.GetBlotterSettings();
    settings.SetActiveBlotter(settings.GetConsolidatedBlotter());
    settings.SetDefaultBlotterTaskProperties(
      BlotterTaskProperties::GetDefault());
    settings.SetDefaultOrderLogProperties(OrderLogProperties::GetDefault());
    auto left_layout = new QVBoxLayout();
    auto book_quote_group_box = new QGroupBox(tr("Venue Quote"));
    book_quote_group_box->setSizePolicy(QSizePolicy::Preferred,
      QSizePolicy::Fixed);
    auto book_quote_layout = new QVBoxLayout(book_quote_group_box);
    auto book_quote_fields_layout = new QFormLayout();
    auto book_quote_venue_box =
      make_venue_box(std::make_shared<LocalVenueModel>(Venue("ARCX")));
    book_quote_fields_layout->addRow(tr("MPID:"), book_quote_venue_box);
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
        book_quote_venue_box, book_quote_price_box,
        book_quote_quantity_box, book_quote_side_box));
    book_quote_layout->addWidget(submit_quote_button, 0, Qt::AlignRight);
    auto book_quote_period_layout = new QFormLayout();
    auto quote_update_period_box = new IntegerBox(m_update_period);
    book_quote_period_layout->addRow(tr("Update Period (ms):"),
      quote_update_period_box);
    book_quote_layout->addLayout(book_quote_period_layout);
    left_layout->addWidget(book_quote_group_box);
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
      m_model.get_model()->get_preview_order()->set(make_limit_order_fields(
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

  void on_quote_timeout() {
    auto bid_quote = make_random_venue_quote(Side::BID);
    m_model.submit_book_quote(bid_quote);
    auto ask_quote = make_random_venue_quote(Side::ASK);
    m_model.submit_book_quote(ask_quote);
    m_technicals_model.update(TimeAndSale(bid_quote.m_timestamp,
      bid_quote.m_quote.m_price, bid_quote.m_quote.m_size,
      TimeAndSale::Condition(), bid_quote.m_venue.get_code().get_data(),
      bid_quote.m_mpid, ""));
    m_technicals_model.update(TimeAndSale(ask_quote.m_timestamp,
      ask_quote.m_quote.m_price, ask_quote.m_quote.m_size,
      TimeAndSale::Condition(), ask_quote.m_venue.get_code().get_data(), "",
      ask_quote.m_mpid));
  }

  void on_book_quote_submit_click(VenueBox* venue_box,
      MoneyBox* price_box, QuantityBox* quantity_box, SideBox* side_box) {
    auto venue_code = venue_box->get_current()->get();
    auto quote = BookQuote(to_text(venue_code).toStdString(), true, venue_code,
      Quote(*price_box->get_current()->get(),
        *quantity_box->get_current()->get(), side_box->get_current()->get()),
        second_clock::local_time());
    m_model.submit_book_quote(quote);
  }

  void on_cancel_order(CancelKeyBindingsModel::Operation operation,
      const Security& security,
      const optional<BookViewWindow::CancelCriteria>& criteria) {
    auto log = QString("%1: Operation:[%2]").
      arg(++m_line_number).
      arg(to_text(operation));
    if(criteria) {
      log += QString(" Order Destination:[%1] Order Price:[%2]").
        arg(QString::fromStdString(criteria->m_destination)).
        arg(to_text(criteria->m_price));
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
  auto key_bindings = std::make_shared<KeyBindingsModel>();
  auto key_bindings_window = KeyBindingsWindow(key_bindings,
    populate_security_query_model(), get_default_additional_tag_database());
  auto book_views = make_local_aggregate_book_view_model();
  auto order_tester = BookViewOrderTester(book_views);
  auto tester = BookViewTester(
    book_views->get_technicals(), book_views, key_bindings_window);
  auto window = BookViewWindow(Ref(tester.m_user_profile),
    populate_security_query_model(), key_bindings,
    std::make_shared<BookViewPropertiesWindowFactory>(),
    std::bind_front(&model_builder, book_views, &tester));
  window.connect_cancel_operation_signal(
    std::bind_front(&BookViewTester::on_cancel_order, &tester));
  window.installEventFilter(&tester);
  window.show();
  tester.show();
  tester.move(
    window.pos().x() + window.frameGeometry().width() + scale_width(10),
    window.pos().y());
  order_tester.show();
  order_tester.move(
    window.pos().x() - order_tester.frameGeometry().width() - scale_width(10),
    window.pos().y());
  application.exec();
}
