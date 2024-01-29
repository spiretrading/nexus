#include "Spire/Ui/SideBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  SideBox::Settings setup() {
    static auto settings = [] {
      auto settings = SideBox::Settings();
      auto cases = std::make_shared<ArrayListModel<Side>>();
      cases->push(Side::BID);
      cases->push(Side::ASK);
      settings.m_cases = std::move(cases);
      return settings;
    }();
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
