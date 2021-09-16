#include "Spire/Ui/SideBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

SideBox* Spire::make_side_box(QWidget* parent) {
  return make_side_box(Side::NONE, parent);
}

SideBox* Spire::make_side_box(Side current, QWidget* parent) {
  return make_side_box(std::make_shared<LocalSideModel>(current), parent);
}

SideBox* Spire::make_side_box(std::shared_ptr<SideModel> model,
    QWidget* parent) {
  auto settings = SideBox::Settings();
  auto cases = std::make_shared<ArrayListModel>();
  cases->push(Side(Side::ASK));
  cases->push(Side(Side::BID));
  settings.m_cases = std::move(cases);
  settings.m_current = std::move(model);
  return new SideBox(settings, parent);
}
