#include "Spire/UI/WindowSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <QApplication>
#include <QMessageBox>
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/Toolbar.hpp"
#include "Spire/UI/ToolbarWindowSettings.hpp"
#include "Spire/UI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::filesystem;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

vector<unique_ptr<WindowSettings>> WindowSettings::Load(
    const UserProfile& userProfile) {
  auto windowSettingsPath = userProfile.GetProfilePath() / "layout.dat";
  vector<unique_ptr<WindowSettings>> windowSettings;
  if(!exists(windowSettingsPath)) {
    return windowSettings;
  }
  try {
    BasicIStreamReader<boost::filesystem::ifstream> reader(
      Initialize(windowSettingsPath, ios::binary));
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    BinaryReceiver<SharedBuffer> receiver(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(windowSettings);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load layout."));
    windowSettings.clear();
  }
  return windowSettings;
}

void WindowSettings::Save(const UserProfile& userProfile) {
  auto windowSettingsPath = userProfile.GetProfilePath() / "layout.dat";
  vector<unique_ptr<WindowSettings>> windowSettings;
  auto widgets = QApplication::topLevelWidgets();
  for(const auto& widget : widgets) {
    auto window = dynamic_cast<PersistentWindow*>(widget);
    if(window != nullptr) {
      if(dynamic_cast<Toolbar*>(window) != nullptr) {
        windowSettings.insert(windowSettings.begin(),
          window->GetWindowSettings());
      } else {
        if(widget->isVisible()) {
          windowSettings.push_back(window->GetWindowSettings());
        }
      }
    }
  }
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    BinarySender<SharedBuffer> sender(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(windowSettings);
    BasicOStreamWriter<boost::filesystem::ofstream> writer(
      Initialize(windowSettingsPath, ios::binary));
    writer.Write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save layout."));
  }
}

WindowSettings::~WindowSettings() {}

string WindowSettings::GetName() const {
  return "";
}
