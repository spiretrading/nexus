#include "Spire/LegacyUI/WindowSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QApplication>
#include <QMessageBox>
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/ToolbarWindowSettings.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;

std::vector<std::unique_ptr<WindowSettings>>
    WindowSettings::Load(const UserProfile& userProfile) {
  auto windowSettingsPath = userProfile.GetProfilePath() / "layout.dat";
  auto windowSettings = std::vector<std::unique_ptr<WindowSettings>>();
  if(!exists(windowSettingsPath)) {
    return windowSettings;
  }
  try {
    auto reader = BasicIStreamReader<std::ifstream>(
      Initialize(windowSettingsPath, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.Read(Store(buffer));
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(windowSettings);
  } catch(const std::exception&) {
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
      if(dynamic_cast<ToolbarWindow*>(window)) {
        windowSettings.insert(
          windowSettings.begin(), window->GetWindowSettings());
      } else if(widget->isVisible()) {
        windowSettings.push_back(window->GetWindowSettings());
      }
    }
  }
  try {
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(windowSettings);
    auto writer = BasicOStreamWriter<std::ofstream>(
      Initialize(windowSettingsPath, std::ios::binary));
    writer.Write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(
      nullptr, QObject::tr("Warning"), QObject::tr("Unable to save layout."));
  }
}

std::string WindowSettings::GetName() const {
  return "";
}
