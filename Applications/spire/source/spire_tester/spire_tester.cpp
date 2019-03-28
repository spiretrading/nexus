#include "spire/spire_tester/spire_tester.hpp"

void Spire::print_test_name(const QString& name) {
  qDebug() << "\n**************************************************";
  qDebug() << name << " PASSED";
  qDebug() << "**************************************************\n";
}
