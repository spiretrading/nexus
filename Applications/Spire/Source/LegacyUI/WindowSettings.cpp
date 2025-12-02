#include "Spire/LegacyUI/WindowSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
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
      init(windowSettingsPath, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(windowSettings);
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
