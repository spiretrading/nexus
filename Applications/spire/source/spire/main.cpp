#include <cstdio>
#include <QApplication>
#include <QFontDatabase>
#include <QtPlugin>
#include "spire/spire/spire_controller.hpp"
#include "spire/version.hpp"

#ifdef _MSC_VER
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif __linux__
  Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

using namespace spire;

void initialize_resources() {
  Q_INIT_RESOURCE(resources);
  QStringList fonts;
  fonts << ":/fonts/Roboto/Roboto-Black.ttf" <<
    ":/fonts/Roboto/Roboto-BlackItalic.ttf" <<
    ":/fonts/Roboto/Roboto-Bold.ttf" <<
    ":/fonts/Roboto/Roboto-BoldItalic.ttf" <<
    ":/fonts/Roboto/RobotoCondensed-Bold.ttf" <<
    ":/fonts/Roboto/RobotoCondensed-BoldItalic.ttf" <<
    ":/fonts/Roboto/RobotoCondensed-Italic.ttf" <<
    ":/fonts/Roboto/RobotoCondensed-Light.ttf" <<
    ":/fonts/Roboto/RobotoCondensed-LightItalic.ttf" <<
    ":/fonts/Roboto/RobotoCondensed-Regular.ttf" <<
    ":/fonts/Roboto/Roboto-Italic.ttf" <<
    ":/fonts/Roboto/Roboto-Light.ttf" <<
    ":/fonts/Roboto/Roboto-LightItalic.ttf" <<
    ":/fonts/Roboto/Roboto-Medium.ttf" <<
    ":/fonts/Roboto/Roboto-MediumItalic.ttf" <<
    ":/fonts/Roboto/Roboto-Regular.ttf" <<
    ":/fonts/Roboto/Roboto-Thin.ttf" <<
    ":/fonts/Roboto/Roboto-ThinItalic.ttf";
  for(auto& font : fonts) {
    QFontDatabase::addApplicationFont(font);
  }
}

int main(int argc, char** argv) {
#ifdef _DEBUG
  std::freopen("stdout.log", "w", stdout);
  std::freopen("stderr.log", "w", stderr);
#endif
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Spire"));
  application->setApplicationVersion(SPIRE_VERSION);
  initialize_resources();
  auto f = QApplication::font();
  f.setStyleStrategy(QFont::PreferAntialias);
  QApplication::setFont(f);
  spire_controller controller;
  controller.open();
  application->exec();
  controller.close();
}
