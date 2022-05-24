#include "Spire/Ui/SideFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace Nexus;
using namespace Spire;

SideFilterPanel* Spire::make_side_filter_panel(QWidget& parent) {
  return make_side_filter_panel(
    std::make_shared<ArrayListModel<Side>>(), parent);
}

SideFilterPanel* Spire::make_side_filter_panel(
    std::shared_ptr<SideListModel> selected_model, QWidget& parent) {
  auto model = std::make_shared<ArrayTableModel>();
  model->push({Side(Side::BID), false});
  model->push({Side(Side::ASK), false});
  for(auto i = 0; i < selected_model->get_size(); ++i) {
    if(selected_model->get(i) == Side::BID) {
      model->set(0, 1, true);
    } else if(selected_model->get(i) == Side::ASK) {
      model->set(1, 1, true);
    }
  }
  return new ClosedFilterPanel(
    std::move(model), QObject::tr("Filter by Side"), parent);
}
