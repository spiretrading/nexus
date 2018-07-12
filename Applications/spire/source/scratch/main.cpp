#include <QApplication>
#include <QTimer>
#include "spire/spire/resources.hpp"
#include "spire/ui/flat_button.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto button = new flat_button("Test Button");
  auto s = button->get_style();
  s.m_background_color = QColor("#FFFFFF");
  s.m_text_color = Qt::red;
  s.m_text_size = 40;
  s.m_font_weight = QFont::Black;
  button->set_style(s);
  auto s2 = button->get_disabled_style();
  s2.m_background_color = QColor("#0000FF");
  button->set_disabled_style(s2);
  button->resize(400, 250);
  button->setDisabled(true);
  button->show();
  application->exec();
}
