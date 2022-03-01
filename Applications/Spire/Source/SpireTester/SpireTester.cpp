#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

void Spire::wait_until(const std::function<bool ()>& predicate) {
  while(!predicate()) {
    QApplication::processEvents(QEventLoop::WaitForMoreEvents);
    QCoreApplication::sendPostedEvents();
  }
}
