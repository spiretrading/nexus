#include "spire/security_input_ui_tester/security_input_tester.hpp"
#include <QTimer>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/security_input/security_input_dialog.hpp"
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
}

void security_input_tester::closeEvent(QCloseEvent* event) {
  m_dialog->close();
}

void security_input_tester::showEvent(QShowEvent* event) {
  QTimer::singleShot(0, this,
    [=] {
      auto positioned = false;
      while(true) {
        m_dialog = new security_input_dialog(m_model);
        if(!positioned) {
          m_dialog->show();
          move(m_dialog->geometry().topRight().x(),
            m_dialog->geometry().topRight().y());
          positioned = true;
        }
        if(m_dialog->exec() == QDialog::Accepted) {
          addItem(QString::fromStdString(
            Nexus::ToString(m_dialog->get_security())));
        } else {
          break;
        }
        delete m_dialog;
      }
      close();
    });
}
