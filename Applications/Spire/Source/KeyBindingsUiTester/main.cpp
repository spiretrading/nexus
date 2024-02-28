#include <QApplication>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QVBoxLayout>
#include "Spire/KeyBindings/AddRegionForm.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/LegacyKeyBindings/HotkeyOverride.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Nexus;
using namespace Spire;

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

class KeyBindingsUiTester : public QWidget {
  public:
    KeyBindingsUiTester(QWidget* parent = nullptr)
      : QWidget(parent) {
      auto add_region_button = make_label_button(tr("Add Region"));
      add_region_button->connect_click_signal(
        std::bind_front(&KeyBindingsUiTester::on_add_region_click, this));
      m_output = new QTextEdit();
      m_output->setReadOnly(true);
      auto left_layout = new QVBoxLayout();
      left_layout->setSpacing(scale_height(10));
      left_layout->addWidget(add_region_button);
      left_layout->addStretch();
      auto layout = new QHBoxLayout(this);
      layout->setSpacing(scale_width(10));
      layout->addLayout(left_layout);
      layout->addWidget(m_output);
      m_add_region_from = new AddRegionForm(populate_regions(), *this);
      m_add_region_from->connect_submit_signal(
        std::bind_front(&KeyBindingsUiTester::on_add_region, this));
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

    void on_add_region_click() {
      m_add_region_from->show();
    }

    void on_add_region(const Region& region) {
      m_output->append(QString("Add Region: %1").arg(to_text(region)));
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
