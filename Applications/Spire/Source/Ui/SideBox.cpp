#include "Spire/Ui/SideBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  SideBox::Settings setup(std::shared_ptr<SideModel> current) {
    auto settings = SideBox::Settings();
    auto cases = std::make_shared<ArrayListModel>();
    cases->push(Side(Side::ASK));
    cases->push(Side(Side::BID));
    settings.m_cases = std::move(cases);
    settings.m_current = std::move(current);
    return settings;
  }
}

SideBox::SideBox(QWidget* parent)
  : SideBox(Side::NONE, parent) {}

SideBox::SideBox(Side current, QWidget* parent)
  : SideBox(std::make_shared<LocalSideModel>(current), parent) {}

SideBox::SideBox(std::shared_ptr<SideModel> model, QWidget* parent)
  : EnumBox(setup(std::move(model)), parent) {}
