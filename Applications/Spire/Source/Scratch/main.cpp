#include <QApplication>
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto promise = QtPromise([] { return 5; });
  auto p = promise.then([] (auto&& result) {
    throw std::exception("Shouldn't be called");
  });
  p.disconnect();
  application->exec();
}
