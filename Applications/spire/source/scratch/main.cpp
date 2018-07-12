#include <QApplication>
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
  button->resize(400, 250);
  button->show();
  application->exec();
}
