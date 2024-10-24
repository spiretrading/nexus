#include "Spire/LegacyUI/WindowSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QScreen>
#include <QWindow>
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"
#include "Spire/Ui/Window.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  auto load_legacy_settings(const UserProfile& userProfile) {
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
      windowSettings.clear();
    }
    return windowSettings;
  }
}

std::vector<std::unique_ptr<WindowSettings>>
    WindowSettings::Load(const UserProfile& userProfile) {
  auto settings = std::vector<std::unique_ptr<WindowSettings>>();
  auto file_path = userProfile.GetProfilePath() / "layout.json";
  if(!std::filesystem::exists(file_path)) {
    auto legacy_path = userProfile.GetProfilePath() / "layout.dat";
    if(std::filesystem::exists(legacy_path)) {
      return load_legacy_settings(userProfile);
    }
    return settings;
  }
  try {
    auto reader = BasicIStreamReader<std::ifstream>(Initialize(file_path));
    auto buffer = SharedBuffer();
    reader.Read(Store(buffer));
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(Store(registry));
    auto receiver = JsonReceiver<SharedBuffer>(Ref(registry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(settings);
  } catch(const std::exception&) {
    settings.clear();
  }
  return settings;
}

void WindowSettings::Save(const UserProfile& userProfile) {
  auto settings_path = userProfile.GetProfilePath() / "layout.json";
  auto settings = std::vector<std::unique_ptr<WindowSettings>>();
  auto widgets = QApplication::topLevelWidgets();
  for(auto& widget : widgets) {
    if(auto window = dynamic_cast<PersistentWindow*>(widget)) {
      if(dynamic_cast<ToolbarWindow*>(window)) {
        settings.insert(settings.begin(), window->GetWindowSettings());
      } else if(widget->isVisible()) {
        settings.push_back(window->GetWindowSettings());
      }
    }
  }
  try {
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(Store(registry));
    auto sender = JsonSender<SharedBuffer>(Ref(registry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(settings);
    auto writer = BasicOStreamWriter<std::ofstream>(Initialize(settings_path));
    writer.Write(buffer);
  } catch(const std::exception&) {
    throw std::runtime_error("Unable to save layout.");
  }
}

std::string WindowSettings::GetName() const {
  return "";
}

void Spire::LegacyUI::restore_geometry(
    QWidget& widget, const QByteArray& geometry) {
  if(geometry.size() < 4) {
    return;
  }
  auto stream = QDataStream(geometry);
  stream.setVersion(QDataStream::Qt_4_0);
  static const auto MAGIC_NUMBER = quint32(0x1D9D0CB);
  auto stored_magic_number = quint32();
  stream >> stored_magic_number;
  if(stored_magic_number != MAGIC_NUMBER) {
    return;
  }
  static const auto CURRENT_MAJOR_VERSION = quint16(3);
  auto major_version = quint16(0);
  auto minor_version = quint16(0);
  stream >> major_version >> minor_version;
  if(major_version > CURRENT_MAJOR_VERSION) {
    return;
  }
  auto restored_frame_geometry = QRect();
  auto restored_geometry = QRect();
  auto restored_normal_geometry = QRect();
  auto restored_screen_number = qint32();
  auto is_maximized = quint8();
  auto is_full_screen = quint8();
  auto restored_screen_width = qint32();
  stream >> restored_frame_geometry >> restored_normal_geometry >>
    restored_screen_number >> is_maximized >> is_full_screen;
  if(major_version > 1) {
    stream >> restored_screen_width;
  }
  if(major_version > 2) {
    stream >> restored_geometry;
  }
  if(restored_screen_number >= QApplication::screens().count()) {
    restored_screen_number =
      QApplication::screens().indexOf(QApplication::primaryScreen());
  }
  auto restored_screen = QApplication::screens()[restored_screen_number];
  auto screen_width = qreal(restored_screen->geometry().width());
  auto width_factor = screen_width / qreal(restored_screen_width);
  auto available_geometry = restored_screen->availableGeometry();
  auto window_state =
    widget.windowState() & ~(Qt::WindowMaximized | Qt::WindowFullScreen);
  if(is_maximized) {
    window_state |= Qt::WindowMaximized;
  }
  if(is_full_screen) {
    window_state |= Qt::WindowFullScreen;
  }
  widget.setWindowState(window_state);
  auto new_geometry = [&] {
    if(major_version > 2) {
      return restored_geometry;
    } else {
      return restored_normal_geometry;
    }
  }();
  new_geometry = QRect(std::floor(new_geometry.x() * width_factor),
    new_geometry.y() * width_factor, new_geometry.width() * width_factor,
    new_geometry.height() * width_factor);
  widget.setGeometry(new_geometry);
  if(auto window = dynamic_cast<Window*>(&widget)) {
    window->move(window->x() - 1, new_geometry.top());
  }
  if(widget.isWindow()) {
    static const auto FRAME_HEIGHT = 20;
    auto top_region = widget.frameGeometry();
    top_region.setHeight(FRAME_HEIGHT);
    auto desktop = QApplication::desktop();
    auto screen_geometry = desktop->screenGeometry(&widget);
    if(!screen_geometry.intersects(top_region)) {
      widget.move(0, 0);
    }
  }
}
