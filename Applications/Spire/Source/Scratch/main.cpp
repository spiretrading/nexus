#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto t = chain(timeout(QColor(0xFFF1F1), milliseconds(250)), QColor(0xFFFFFF));
  application.exec();
}
