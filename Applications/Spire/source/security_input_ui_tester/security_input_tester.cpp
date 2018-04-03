#include "spire/security_input_ui_tester/security_input_tester.hpp"
#include <thread>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Nexus;
using namespace spire;

security_input_tester::security_input_tester(QWidget* parent)
    : QListWidget(parent) {
  setFixedSize(scale(100, 250));
  m_model.add(SecurityInfo(
    Security("MSFT", DefaultMarkets::NASDAQ(), DefaultCountries::US()),
    "Microsoft Corp", "Software"));
  m_model.add(SecurityInfo(
    Security("MG", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Magna International Inc.", "Automotive, probably"));
  m_model.add(SecurityInfo(
    Security("MFC", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Manulife Financial Corporation", "Finance"));
  m_model.add(SecurityInfo(
    Security("MX", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Methanex Corporation", ""));
  m_model.add(SecurityInfo(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Metro Inc.", ""));
  m_model.add(SecurityInfo(
    Security("MON", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Monsanto Co.", ""));
  m_model.add(SecurityInfo(
    Security("MS", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Morgan Stanley", "Finance"));
  m_dialog = new security_input_dialog(m_model);
  connect(m_dialog, &QDialog::rejected, [=] { close(); });
  connect(m_dialog, &QDialog::accepted, [=] { run_dialog(); });
  run_dialog();
  clear();
  setGeometry(m_dialog->pos().x() + width() * 2, m_dialog->pos().y(), 0, 0);
}

security_input_tester::~security_input_tester() {
  delete m_dialog;
}

void security_input_tester::add_security(const Security& security) {
  addItem(QString::fromStdString(Nexus::ToString(security)));
}

void security_input_tester::closeEvent(QCloseEvent* event) {
  m_dialog->close();
}

void security_input_tester::run_dialog() {
  m_dialog->show();
  add_security(m_dialog->get_security());
}
