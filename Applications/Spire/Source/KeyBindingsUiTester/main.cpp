#include <QApplication>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QVBoxLayout>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

std::shared_ptr<ComboBox::QueryModel> populate_security_query_model() {
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
  auto model = std::make_shared<LocalComboBoxQueryModel>();
  for(auto security_info : security_infos) {
    model->add(to_text(security_info.m_security).toLower(), security_info);
    model->add(
      QString::fromStdString(security_info.m_name).toLower(), security_info);
  }
  return model;
}

class MaxFloorSchema : public AdditionalTagSchema {
  public:
    MaxFloorSchema()
      : AdditionalTagSchema("MaxFloor", 111) {}

    std::unique_ptr<CanvasNode> make_canvas_node(
        const optional<Nexus::Tag::Type>& value) const {
      return LinkedNode::SetReferent(MaxFloorNode(), "security");
    }
};

const AdditionalTagDatabase& get_default_additional_tag_database() {
  static auto database = [] {
    auto database = AdditionalTagDatabase();
    database.add(Region::Global(), std::make_shared<MaxFloorSchema>());
    return database;
  }();
  return database;
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("KeyBindings Ui Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto key_bindings =
    std::make_shared<KeyBindingsModel>(GetDefaultMarketDatabase());
  auto tasks = make_default_order_task_nodes();
  for(auto& task : tasks) {
    key_bindings->get_order_task_arguments()->push(to_order_task_arguments(
      *task, GetDefaultMarketDatabase(), GetDefaultDestinationDatabase()));
  }
  auto window = KeyBindingsWindow(key_bindings, populate_security_query_model(),
    GetDefaultCountryDatabase(), GetDefaultMarketDatabase(),
    GetDefaultDestinationDatabase(), get_default_additional_tag_database());
  window.show();
  auto hotkey_override = HotkeyOverride();
  application.exec();
}
