#include "Spire/UI/WindowSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QApplication>
#include <QMessageBox>
#include "Spire/Canvas/OrderExecutionNodes/TickerPortfolioNode.hpp"
#include "Spire/Canvas/Types/TickerType.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/Toolbar.hpp"
#include "Spire/UI/ToolbarWindowSettings.hpp"
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::UI;

namespace {
  bool TryLoad(SharedBuffer& buffer,
      Out<std::vector<std::unique_ptr<WindowSettings>>> windowSettings) {
    auto attempt =
      [&] (TypeRegistry<BinarySender<SharedBuffer>>& typeRegistry) {
        try {
          auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
          receiver.set(Ref(buffer));
          auto loaded = std::vector<std::unique_ptr<WindowSettings>>();
          receiver.shuttle(loaded);
          *windowSettings = std::move(loaded);
          return true;
        } catch(const std::exception&) {
          return false;
        }
      };
    auto currentRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(currentRegistry));
    if(attempt(currentRegistry)) {
      return true;
    }
    auto legacyRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    legacyRegistry.add<TickerNode>("Spire.SecurityNode");
    legacyRegistry.add<TickerType>("Spire.SecurityType");
    legacyRegistry.add<TickerPortfolioNode>("Spire.SecurityPortfolioNode");
    RegisterSpireTypes(out(legacyRegistry));
    return attempt(legacyRegistry);
  }
}

std::vector<std::unique_ptr<WindowSettings>>
    WindowSettings::Load(const UserProfile& userProfile) {
  auto windowSettingsPath = userProfile.GetProfilePath() / "layout.dat";
  auto windowSettings = std::vector<std::unique_ptr<WindowSettings>>();
  if(!exists(windowSettingsPath)) {
    return windowSettings;
  }
  auto isLoaded = [&] {
    try {
      auto reader = BasicIStreamReader<std::ifstream>(
        init(windowSettingsPath, std::ios::binary));
      auto buffer = SharedBuffer();
      reader.read(out(buffer));
      return TryLoad(buffer, out(windowSettings));
    } catch(const std::exception&) {
      return false;
    }
  }();
  if(!isLoaded) {
    QMessageBox::warning(
      nullptr, QObject::tr("Warning"), QObject::tr("Unable to load layout."));
    windowSettings.clear();
  }
  return windowSettings;
}

void WindowSettings::Save(const UserProfile& userProfile) {
  auto windowSettingsPath = userProfile.GetProfilePath() / "layout.dat";
  auto windowSettings = std::vector<std::unique_ptr<WindowSettings>>();
  auto widgets = QApplication::topLevelWidgets();
  for(auto& widget : widgets) {
    if(auto window = dynamic_cast<PersistentWindow*>(widget)) {
      if(dynamic_cast<Toolbar*>(window)) {
        windowSettings.insert(
          windowSettings.begin(), window->GetWindowSettings());
      } else if(widget->isVisible()) {
        windowSettings.push_back(window->GetWindowSettings());
      }
    }
  }
  try {
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    auto buffer = SharedBuffer();
    sender.set(Ref(buffer));
    sender.shuttle(windowSettings);
    auto writer = BasicOStreamWriter<std::ofstream>(
      init(windowSettingsPath, std::ios::binary));
    writer.write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(
      nullptr, QObject::tr("Warning"), QObject::tr("Unable to save layout."));
  }
}

std::string WindowSettings::GetName() const {
  return "";
}
