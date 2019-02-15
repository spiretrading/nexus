#include <QApplication>
#include <QWidget>
#include "spire/spire/qt_promise.hpp"
#include "spire/spire/resources.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  w->resize(500, 500);
  w->show();
  auto done = false;
  auto promise = Spire::make_qt_promise([&] {
    auto p = Spire::make_qt_promise([&] { return 2; });
    p.then([&] (auto result) { done = true; });
    while(!done) {
      qDebug() << "not done";
    }
    return 2;
  });
  promise.then([=] (auto result) { qDebug() << "result is: " << result; });
  application->exec();
}
