#include "Spire/Ui/SideBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  std::shared_ptr<ListModel> populate_data() {
    auto list_model = std::make_shared<ArrayListModel>();
    list_model->push(Side(Side::ASK));
    list_model->push(Side(Side::BID));
    return list_model;
  }
}

SideBox::SideBox(QWidget* parent)
  : SideBox(std::make_shared<LocalValueModel<optional<int>>>(), parent) {}

SideBox::SideBox(std::shared_ptr<ValueModel<optional<int>>> model,
  QWidget* parent)
  : DropDownBoxAdaptor(populate_data(), model, parent) {}
