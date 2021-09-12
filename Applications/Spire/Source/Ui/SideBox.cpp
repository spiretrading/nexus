#include "Spire/Ui/SideBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

SideBox::SideBox(QWidget* parent)
  : SideBox(Side::NONE, parent) {}

SideBox::SideBox(Side current, QWidget* parent)
  : SideBox(std::make_shared<LocalSideModel>(current), parent) {}

SideBox::SideBox(std::shared_ptr<SideModel> current, QWidget* parent)
  : EnumBox(setup(std::move(current)), parent) {}

SideBox::Settings SideBox::setup(std::shared_ptr<SideModel> current) {
  auto settings = Settings();
  auto model = std::make_shared<ArrayListModel>();
  model->push(Side(Side::ASK));
  model->push(Side(Side::BID));
  settings.m_cases = std::move(model);
  settings.m_current = std::move(current);
  return settings;
}
