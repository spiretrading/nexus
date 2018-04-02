#include <QApplication>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/security_input_ui_tester/security_input_tester.hpp"
#include "spire/spire/resources.hpp"

using namespace Nexus;
using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Security Input UI Tester"));
  initialize_resources();
  local_security_input_model model;
  model.add(SecurityInfo(
    Security("MSFT", DefaultMarkets::NASDAQ(), DefaultCountries::US()),
    "Microsoft Corp", "Software"));
  model.add(SecurityInfo(
    Security("MG", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Magna International Inc.", "Automotive, probably"));
  model.add(SecurityInfo(
    Security("MFC", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Manulife Financial Corporation", "Finance"));
  model.add(SecurityInfo(
    Security("MX", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Methanex Corporation", ""));
  model.add(SecurityInfo(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Metro Inc.", ""));
  model.add(SecurityInfo(
    Security("MON", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Monsanto Co.", ""));
  model.add(SecurityInfo(
    Security("MS", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Morgan Stanley", "Finance"));
  security_input_dialog dialog(model);
  dialog.exec();
  security_input_tester tester;
  tester.setGeometry(dialog.pos().x(),
    dialog.pos().y() + dialog.height() + 100, 0, 0);
  tester.add_security(dialog.get_security());
  tester.show();
  application->exec();
}
