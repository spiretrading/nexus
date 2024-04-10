#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsForm.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsPage.hpp"
#include "Spire/KeyBindings/TaskKeysTableView.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto populate_order_task_arguments() {
    auto arguments = std::make_shared<ArrayListModel<OrderTaskArguments>>();
    arguments->push({"Test1",
      Region(GetDefaultMarketDatabase().FromCode(DefaultMarkets::TSX())),
      "ALPHA", OrderType::MARKET, Side::ASK, Quantity(1),
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

  auto populate_region_box_model() {
    auto securities = std::vector<std::pair<std::string, std::string>>{
      {"MSFT.NSDQ", "Microsoft Corporation"},
      {"MG.TSX", "Magna International Inc."},
      {"MRU.TSX", "Metro Inc."},
      {"MFC.TSX", "Manulife Financial Corporation"},
      {"MX.TSX", "Methanex Corporation"},
      {"TSO.ASX", "Tesoro Resources Limited"}};
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto& security_info : securities) {
      auto security = *ParseWildCardSecurity(security_info.first,
        GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
      auto region = Region(security);
      region.SetName(security_info.second);
      model->add(to_text(security).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& market : GetDefaultMarketDatabase().GetEntries()) {
      auto region = Region(market);
      region.SetName(market.m_description);
      model->add(to_text(MarketToken(market.m_code)).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& country : GetDefaultCountryDatabase().GetEntries()) {
      auto region = Region(country.m_code);
      region.SetName(country.m_name);
      model->add(to_text(country.m_code).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    return model;
  }
}

KeyBindingsWindow::KeyBindingsWindow(
    std::shared_ptr<KeyBindingsModel> key_bindings,
    const CountryDatabase& countries, const MarketDatabase& markets,
    QWidget* parent)
    : Window(parent),
      m_key_bindings(std::move(key_bindings)) {
  setWindowTitle(tr("Key Bindings"));
  set_svg_icon(":/Icons/key-bindings.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  auto navigation_view = new NavigationView();
  navigation_view->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto task_keys_page = new QWidget();
  task_keys_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto task_key_table_view = make_task_keys_table_view(populate_order_task_arguments(),
    populate_region_box_model(), GetDefaultDestinationDatabase(), markets);
  enclose(*task_keys_page, *task_key_table_view);
  navigation_view->add_tab(*task_keys_page, tr("Task Keys"));
  auto cancel_keys_page =
    new CancelKeyBindingsForm(m_key_bindings->get_cancel_key_bindings());
  cancel_keys_page->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  navigation_view->add_tab(*cancel_keys_page, tr("Cancel Keys"));
  auto interactions_page =
    new InteractionsPage(m_key_bindings, countries, markets);
  interactions_page->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  navigation_view->add_tab(*interactions_page, tr("Interactions"));
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
