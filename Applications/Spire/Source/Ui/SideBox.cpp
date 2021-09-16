#include "Spire/Ui/SideBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  SideBox::Settings setup() {
    static auto settings = SideBox::Settings();
    if(!settings.m_cases) {
      auto cases = std::make_shared<ArrayListModel>();
      cases->push(Side(Side::BID));
      cases->push(Side(Side::ASK));
      settings.m_cases = std::move(cases);
    }
    return settings;
  }
}

SideBox* Spire::make_side_box(QWidget* parent) {
  return make_side_box(Side::BID, parent);
}

SideBox* Spire::make_side_box(Side current, QWidget* parent) {
  return make_side_box(std::make_shared<LocalSideModel>(current), parent);
}

SideBox* Spire::make_side_box(std::shared_ptr<SideModel> model,
    QWidget* parent) {
  auto settings = setup();
  settings.m_current = std::move(model);
  return new SideBox(std::move(settings), parent);
}
