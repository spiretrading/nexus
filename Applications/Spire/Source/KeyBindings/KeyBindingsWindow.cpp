#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsForm.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/KeyBindings/OrderTasksKeyBindingsForm.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_interactions_page() {
    auto interactions_form = new InteractionsKeyBindingsForm(Region::Global(),
      std::make_shared<InteractionsKeyBindingsModel>());
    interactions_form->setMinimumWidth(scale_width(384));
    interactions_form->setMaximumWidth(scale_width(480));
    auto scroll_box_body = new QWidget();
    scroll_box_body->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    auto center_layout = make_vbox_layout();
    center_layout->addWidget(interactions_form);
    center_layout->addStretch(1);
    auto scroll_box_body_layout = make_hbox_layout(scroll_box_body);
    scroll_box_body_layout->addStretch(0);
    scroll_box_body_layout->addLayout(center_layout, 1);
    scroll_box_body_layout->addStretch(0);
    auto scroll_box = new ScrollBox(scroll_box_body);
    scroll_box->setFocusPolicy(Qt::NoFocus);
    scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*scroll_box, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
    });
    auto interactions_page = new QWidget();
    enclose(*interactions_page, *scroll_box);
    interactions_page->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    return interactions_page;
  }

  auto populate_region_box_model() {
    auto securities = std::vector<std::pair<std::string, std::string>>{
      {"MSFT.NSDQ", "Microsoft Corporation"},
      {"MG.TSX", "Magna International Inc."},
      {"MRU.TSX", "Metro Inc."},
      {"MFC.TSX", "Manulife Financial Corporation"},
      {"MX.TSX", "Methanex Corporation"},
      {"TSO.ASX", "Tesoro Resources Limited"}};
    auto markets = std::vector<MarketCode>{DefaultMarkets::NSEX(),
      DefaultMarkets::ISE(), DefaultMarkets::CSE(), DefaultMarkets::TSX(),
      DefaultMarkets::TSXV(), DefaultMarkets::BOSX()};
    auto countries = std::vector<CountryCode>{DefaultCountries::US(),
      DefaultCountries::CA(), DefaultCountries::AU(), DefaultCountries::JP(),
      DefaultCountries::CN()};
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto& security_info : securities) {
      auto security = *ParseWildCardSecurity(security_info.first,
        GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
      auto region = Region(security);
      region.SetName(security_info.second);
      model->add(to_text(security).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& market_code : markets) {
      auto market = GetDefaultMarketDatabase().FromCode(market_code);
      auto region = Region(market);
      region.SetName(market.m_description);
      model->add(to_text(MarketToken(market.m_code)).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& country : countries) {
      auto region = Region(country);
      region.SetName(
        GetDefaultCountryDatabase().FromCode(country).m_name);
      model->add(to_text(country).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    return model;
  }

  auto populate_order_task_arguments() {
    auto arguments = std::make_shared<ArrayListModel<OrderTaskArguments>>();
    arguments->push({"Test1",
      Region(*ParseWildCardSecurity(
        "MSFT.NSDQ", GetDefaultMarketDatabase(), GetDefaultCountryDatabase())),
      "NASDAQ", OrderType::MARKET, Side::ASK, Quantity(1),
      TimeInForce(TimeInForce::Type::DAY), {}, QKeySequence("Ctrl+F4")});
    arguments->push({"Test2",
      Region(GetDefaultMarketDatabase().FromCode(DefaultMarkets::TSX())),
      "TSX", OrderType::STOP, Side::ASK, Quantity(10),
      TimeInForce(TimeInForce::Type::DAY), {}, QKeySequence("Ctrl+Alt+S")});
    arguments->push({"Test3",
      Region(DefaultCountries::US()), "NYSE",
      OrderType::MARKET, Side::BID, Quantity(20),
      TimeInForce(TimeInForce::Type::IOC), {}, QKeySequence("F3")});
    return arguments;
  }
}

KeyBindingsWindow::KeyBindingsWindow(QWidget* parent)
    : Window(parent) {
  setWindowTitle(tr("Key Bindings"));
  set_svg_icon(":/Icons/key-bindings.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  auto navigation_view = new NavigationView();
  navigation_view->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  auto task_keys_page = new OrderTasksKeyBindingsForm(
    populate_region_box_model(), populate_order_task_arguments(),
    GetDefaultDestinationDatabase(), GetDefaultMarketDatabase());
  task_keys_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  navigation_view->add_tab(*task_keys_page, tr("Task Keys"));
  auto cancel_keys_page =
    new CancelKeyBindingsForm(std::make_shared<CancelKeyBindingsModel>());
  cancel_keys_page->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  navigation_view->add_tab(*cancel_keys_page, tr("Cancel Keys"));
  navigation_view->add_tab(*make_interactions_page(), tr("Interactions"));
  auto buttons_body = new QWidget();
  auto buttons_body_layout = make_hbox_layout(buttons_body);
  buttons_body_layout->setSpacing(scale_width(8));
  buttons_body_layout->addStretch(1);
  auto cancel_button = make_label_button(tr("Cancel"));
  cancel_button->setFixedWidth(scale_width(100));
  cancel_button->connect_click_signal(
    std::bind_front(&KeyBindingsWindow::on_cancel, this));
  buttons_body_layout->addWidget(cancel_button);
  auto done_button = make_label_button(tr("Done"));
  done_button->setFixedWidth(scale_width(100));
  done_button->connect_click_signal(
    std::bind_front(&KeyBindingsWindow::on_done, this));
  buttons_body_layout->addWidget(done_button);
  auto buttons = new Box(buttons_body);
  update_style(*buttons, [] (auto& style) {
    style.get(Any()).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xE0E0E0))).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(8))).
      set(PaddingTop(scale_height(29)));
  });
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  body_layout->addWidget(navigation_view);
  body_layout->addWidget(buttons);
  set_body(body);
  resize(scale(928, 640));
}

void KeyBindingsWindow::on_cancel() {
  close();
}

void KeyBindingsWindow::on_done() {
  close();
}
