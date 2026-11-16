#include <QApplication>
#include <QtPlugin>
#include <QWidget>

#if defined(_WIN32) && defined(QT_STATIC)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Blotter UI Tester"));
  application.setQuitOnLastWindowClosed(true);
  auto window = QWidget();
  window.setWindowTitle(QObject::tr("Blotter"));
  window.show();
  return application.exec();
}
