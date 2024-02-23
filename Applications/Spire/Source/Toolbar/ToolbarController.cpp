#include "Spire/Toolbar/ToolbarController.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  std::shared_ptr<WindowSettings> clone(const WindowSettings& settings) {
    auto type_registry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(type_registry));
    auto sender = BinarySender<SharedBuffer>(Ref(type_registry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(&settings);
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(type_registry));
    receiver.SetSource(Ref(buffer));
    auto cloned_settings = std::shared_ptr<WindowSettings>();
    receiver.Shuttle(cloned_settings);
    return cloned_settings;
  }
}

ToolbarController::ToolbarController(Ref<UserProfile> user_profile)
  : m_user_profile(user_profile.Get()) {}

void ToolbarController::open() {
  if(m_toolbar_window) {
    return;
  }
  auto account =
    m_user_profile->GetServiceClients().GetServiceLocatorClient().GetAccount();
  auto roles = m_user_profile->
    GetServiceClients().GetAdministrationClient().LoadAccountRoles(account);
  auto recently_closed_windows =
    std::make_shared<ArrayListModel<std::shared_ptr<WindowSettings>>>();
  for(auto& recently_closed_window :
      m_user_profile->GetRecentlyClosedWindows()) {
    recently_closed_windows->push(clone(*recently_closed_window));
  }
  auto pinned_blotters = std::make_shared<ArrayListModel<BlotterModel*>>();
  for(auto& blotter : m_user_profile->GetBlotterSettings().GetAllBlotters()) {
    pinned_blotters->push(&*blotter);
  }
  m_toolbar_window = std::make_unique<ToolbarWindow>(
    account, roles, recently_closed_windows, pinned_blotters);
  m_toolbar_window->show();
}

void ToolbarController::close() {
  if(!m_toolbar_window) {
    return;
  }
  m_toolbar_window->close();
  auto window = m_toolbar_window.release();
  window->deleteLater();
  m_toolbar_window = nullptr;
}
