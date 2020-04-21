#include "Spire/SecurityInputUiTester/SecurityInputTester.hpp"
#include <QTimer>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/SecurityInput/SecurityInputDialog.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

SecurityInputTester::SecurityInputTester(QWidget* parent)
    : QListWidget(parent) {
  setFixedSize(scale(100, 250));
  m_model.add(SecurityInfo(
    Security("MSFT", DefaultMarkets::NASDAQ(), DefaultCountries::US()),
    "Microsoft Corp", "Software", 100));
  m_model.add(SecurityInfo(
    Security("MG", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Magna International Inc.", "Automotive, probably", 100));
  m_model.add(SecurityInfo(
    Security("MFC", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Manulife Financial Corporation", "Finance", 100));
  m_model.add(SecurityInfo(
    Security("MX", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Methanex Corporation", "", 100));
  m_model.add(SecurityInfo(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Metro Inc.", "", 100));
  m_model.add(SecurityInfo(
    Security("MON", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Monsanto Co.", "", 100));
  m_model.add(SecurityInfo(
    Security("MS", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Morgan Stanley", "Finance", 100));
}

void SecurityInputTester::closeEvent(QCloseEvent* event) {
  m_dialog->close();
}

void SecurityInputTester::showEvent(QShowEvent* event) {
  QTimer::singleShot(0, this,
    [=] {
      auto positioned = false;
      while(true) {
        m_dialog = new SecurityInputDialog(Ref(m_model));
        if(!positioned) {
          m_dialog->show();
          move(m_dialog->geometry().topRight().x(),
            m_dialog->geometry().topRight().y());
          positioned = true;
        }
        if(m_dialog->exec() == QDialog::Accepted) {
          auto item_delegate = CustomVariantItemDelegate(m_dialog);
          addItem(item_delegate.displayText(
            QVariant::fromValue(m_dialog->get_security()), QLocale()));
        } else {
          break;
        }
        delete m_dialog;
      }
      close();
    });
}
