#include "Spire/Spire/Resources.hpp"
#include <cstdio>
#include <QApplication>
#include <QFontDatabase>
#include <QtPlugin>

#ifdef _MSC_VER
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif __linux__
  Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

Q_IMPORT_PLUGIN(QGifPlugin);
Q_IMPORT_PLUGIN(QSvgIconPlugin);

void local_resources() {
  Q_INIT_RESOURCE(Resources);
  auto fonts = QStringList();
  fonts << ":/Fonts/Roboto/Roboto-Black.ttf" <<
    ":/Fonts/Roboto/Roboto-BlackItalic.ttf" <<
    ":/Fonts/Roboto/Roboto-Bold.ttf" <<
    ":/Fonts/Roboto/Roboto-BoldItalic.ttf" <<
    ":/Fonts/Roboto/RobotoCondensed-Bold.ttf" <<
    ":/Fonts/Roboto/RobotoCondensed-BoldItalic.ttf" <<
    ":/Fonts/Roboto/RobotoCondensed-Italic.ttf" <<
    ":/Fonts/Roboto/RobotoCondensed-Light.ttf" <<
    ":/Fonts/Roboto/RobotoCondensed-LightItalic.ttf" <<
    ":/Fonts/Roboto/RobotoCondensed-Regular.ttf" <<
    ":/Fonts/Roboto/Roboto-Italic.ttf" <<
    ":/Fonts/Roboto/Roboto-Light.ttf" <<
    ":/Fonts/Roboto/Roboto-LightItalic.ttf" <<
    ":/Fonts/Roboto/Roboto-Medium.ttf" <<
    ":/Fonts/Roboto/Roboto-MediumItalic.ttf" <<
    ":/Fonts/Roboto/Roboto-Regular.ttf" <<
    ":/Fonts/Roboto/Roboto-Thin.ttf" <<
    ":/Fonts/Roboto/Roboto-ThinItalic.ttf";
  for(auto& font : fonts) {
    QFontDatabase::addApplicationFont(font);
  }
  auto f = QApplication::font();
  f.setStyleStrategy(QFont::PreferAntialias);
  QApplication::setFont(f);
}

void Spire::initialize_resources() {
#ifdef _DEBUG
  std::freopen("stdout.log", "w", stdout);
  std::freopen("stderr.log", "w", stderr);
#endif
  local_resources();
}
