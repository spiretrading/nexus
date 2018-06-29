#include <Beam/Threading/TimerThreadPool.hpp>
#include <QApplication>
#include "spire/spire/resources.hpp"
#include "spire/book_view/book_view_properties_dialog.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Book View UI Tester"));
  initialize_resources();
  auto dialog = new book_view_properties_dialog(book_view_properties(),
    Nexus::Security());
  dialog->show();
  application->exec();
}
