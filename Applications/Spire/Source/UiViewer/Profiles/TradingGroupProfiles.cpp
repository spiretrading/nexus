#include "Spire/UiViewer/Profiles/TradingGroupProfiles.hpp"
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Ui/TradingGroupBox.hpp"
#include "Spire/Ui/TradingGroupFilterPanel.hpp"
#include "Spire/Ui/TradingGroupListBox.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Beam;
using namespace Spire;

namespace {
  std::shared_ptr<TradingGroupQueryModel> populate_trading_group_query_model() {
    auto model = std::make_shared<LocalQueryModel<DirectoryEntry>>();
    auto add = [&] (unsigned int id, const std::string& name) {
      model->add(QString::fromStdString(name),
        DirectoryEntry::make_directory(id, name));
    };
    add(1, "G01");
    add(2, "G11");
    add(3, "G15");
    add(4, "M13");
    add(5, "M15");
    add(6, "M20");
    add(7, "O26");
    add(8, "O34");
    add(9, "O88");
    return model;
  }
}

UiProfile Spire::make_trading_group_box_profile() {
  return setup_combo_box_profile("TradingGroupBox", "G01",
    populate_trading_group_query_model(),
    [] (const DirectoryEntry& entry) { return entry; },
    [] (auto source, auto current) {
      return make_trading_group_box(source, current);
    });
}

UiProfile Spire::make_trading_group_filter_panel_profile() {
  return setup_open_filter_panel_profile("TradingGroupFilterPanel", [] {
    return make_trading_group_filter_panel(
      populate_trading_group_query_model());
  });
}

UiProfile Spire::make_trading_group_list_box_profile() {
  return setup_tag_combo_box_profile("TradingGroupListBox", [] {
    return make_trading_group_list_box(populate_trading_group_query_model());
  });
}
