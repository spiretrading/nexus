#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsForm.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsPage.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto populate_regions() {
    auto countries = std::vector<CountryCode>{DefaultCountries::US(),
      DefaultCountries::CA()};
    auto markets = std::vector<MarketCode>{DefaultMarkets::NSEX(),
      DefaultMarkets::ISE(), DefaultMarkets::CSE(), DefaultMarkets::TSX(),
      DefaultMarkets::TSXV(), DefaultMarkets::BOSX(), DefaultMarkets::CBOE(),
      DefaultMarkets::HKEX(), DefaultMarkets::ASX()};
    auto regions = std::make_shared<ArrayListModel<Region>>();
    for(auto& country : countries) {
      auto region = Region(country);
      region.SetName(GetDefaultCountryDatabase().FromCode(country).m_name);
      regions->push(region);
    }
    for(auto& market_code : markets) {
      auto market = GetDefaultMarketDatabase().FromCode(market_code);
      auto region = Region(market);
      region.SetName(market.m_description);
      regions->push(region);
    }
    return regions;
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
  auto task_keys_page = new QWidget();
  task_keys_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  navigation_view->add_tab(*task_keys_page, tr("Task Keys"));
  auto cancel_keys_page =
    new CancelKeyBindingsForm(std::make_shared<CancelKeyBindingsModel>());
  cancel_keys_page->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  navigation_view->add_tab(*cancel_keys_page, tr("Cancel Keys"));
  auto interactions_page = new InteractionsPage(populate_regions(),
    std::make_shared<RegionInteractionsMap>(
      std::make_shared<InteractionsKeyBindingsModel>()));
  interactions_page->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
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
