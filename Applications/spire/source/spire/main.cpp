#include <cstdio>
#include <QApplication>
#include <QFontDatabase>
#include <QtPlugin>
#include "spire/version.hpp"
#include "spire/spire/spire_controller.hpp"

#ifdef _MSC_VER
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif __linux__
  Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

using namespace spire;

inline void initialize_resources() {}

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
