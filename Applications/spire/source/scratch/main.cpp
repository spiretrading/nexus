#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include "spire/spire/resources.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/title_bar.hpp"
#include "spire/ui/window.hpp"

using namespace spire;

class test_window : public QWidget {
  public:
    test_window(QWidget* parent = nullptr)
        : QWidget(parent) {
      // The window title bar and drop shadow was the reason for the margins,
      // so remove it
      setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
      setBaseSize(600, 480);
      auto ds = new drop_shadow(this);
      setStyleSheet("background-color: aqua;");
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins(50, 50, 50, 50);
      auto widget = new QWidget(this);
      widget->setFixedSize(300, 240);
      widget->setStyleSheet("background-color: blue;");
      layout->addWidget(widget);
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  test_window test;
  test.show();
  application->exec();
}
