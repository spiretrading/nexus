#include <QApplication>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QVBoxLayout>
#include "Spire/KeyBindings/AddRegionForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsPage.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/LegacyKeyBindings/HotkeyOverride.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto modifiers = std::array<Qt::KeyboardModifier, 4>{
    Qt::NoModifier, Qt::ShiftModifier, Qt::ControlModifier, Qt::AltModifier};

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

class KeyBindingsUiTester : public QWidget {
  public:
    KeyBindingsUiTester(QWidget* parent = nullptr)
      : QWidget(parent) {
      auto add_region_button = make_label_button(tr("Add Region"));
      add_region_button->connect_click_signal(
        std::bind_front(&KeyBindingsUiTester::on_add_region_click, this));
      auto interactions_button = make_label_button(tr("Interactions"));
      interactions_button->connect_click_signal(
        std::bind_front(&KeyBindingsUiTester::on_interactions_click, this));
      auto left_layout = new QVBoxLayout();
      left_layout->setSpacing(scale_height(10));
      left_layout->addWidget(add_region_button);
      left_layout->addWidget(interactions_button);
      left_layout->addStretch();
      m_output = new QTextEdit();
      m_output->setReadOnly(true);
      auto layout = new QHBoxLayout(this);
      layout->setSpacing(scale_width(10));
      layout->addLayout(left_layout);
      layout->addWidget(m_output);
      m_add_region_from = new AddRegionForm(populate_regions(), *this);
      m_add_region_from->connect_submit_signal(
        std::bind_front(&KeyBindingsUiTester::on_add_region, this));
      m_interactions_page = new InteractionsPage(populate_regions(),
        std::make_shared<RegionInteractionsMap>("Global",
          std::make_shared<InteractionsKeyBindingsModel>()));
      m_interactions_page->connect_add_signal(
        std::bind_front(&KeyBindingsUiTester::on_add_interactions, this));
      m_interactions_page->connect_delete_signal(
        std::bind_front(&KeyBindingsUiTester::on_delete_interactions, this));
      auto region_interactions =
        m_interactions_page->get_region_interactions()->Begin();
      print_interactions_update(std::get<0>(*region_interactions),
        *std::get<1>(*region_interactions));
    }

  protected:
    bool eventFilter(QObject* object, QEvent* event) override {
      if(event->type() == QEvent::Close) {
        QApplication::quit();
      }
      return QWidget::eventFilter(object, event);
    }

  private:
    QTextEdit* m_output;
    AddRegionForm* m_add_region_from;
    InteractionsPage* m_interactions_page;

    void print_interactions_update(const Region& region,
        const InteractionsKeyBindingsModel& interactions) {
      auto info_template = QString("Update [%1] of the region [%2]: %3");
      interactions.get_default_quantity()->connect_update_signal(
        [=] (auto& value) {
          m_output->append(info_template.
            arg("the default quantity").arg(to_text(region)).
            arg(to_text(value)));
        });
      for(auto modifier : modifiers) {
        interactions.get_quantity_increment(modifier)->connect_update_signal(
          [=] (auto& value) {
            m_output->append(info_template.
              arg("the quantity increment").arg(to_text(region)).
              arg(to_text(value)));
          });
        interactions.get_price_increment(modifier)->connect_update_signal(
          [=] (auto& value) {
            m_output->append(info_template.
              arg("the price increment").arg(to_text(region)).
              arg(to_text(value)));
          });
      }
      interactions.is_cancel_on_fill()->connect_update_signal(
        [=] (auto& value) {
          m_output->append(info_template.
            arg("cancel on fill").arg(to_text(region)).arg(value));
        });
    }

    void on_add_region_click() {
      m_add_region_from->show();
    }

    void on_interactions_click() {
      m_interactions_page->show();
    }

    void on_add_region(const Region& region) {
      m_output->append(QString("Add region: %1").arg(to_text(region)));
    }

    void on_add_interactions(const Region& region,
        const std::shared_ptr<InteractionsKeyBindingsModel>& interactions) {
      m_output->append(QString("Add interactions: %1").arg(to_text(region)));
      print_interactions_update(region, *interactions);
    }

    void on_delete_interactions(const Region& region) {
      m_output->append(QString("Delete Interactions: %1").arg(to_text(region)));
    }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("KeyBindings Ui Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto key_bindings_window = KeyBindingsWindow();
  key_bindings_window.show();
  auto tester = KeyBindingsUiTester();
  key_bindings_window.installEventFilter(&tester);
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.resize(scale(600, 300));
  tester.move(key_bindings_window.x() +
    key_bindings_window.frameGeometry().width() + scale_width(10),
    key_bindings_window.y());
  tester.show();
  auto hotkey_override = HotkeyOverride();
  application.exec();
}
