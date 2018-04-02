#include "spire/security_input_ui_tester/security_input_tester.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Nexus;
using namespace spire;

security_input_tester::security_input_tester(QWidget* parent)
    : QListWidget(parent) {
  setFixedSize(scale(100, 250));
  run_dialog();
}

void security_input_tester::add_security(const Security& security) {
  addItem(QString::fromStdString(security.GetSymbol()) + "." +
    QString(security.GetMarket().GetData()));
}
