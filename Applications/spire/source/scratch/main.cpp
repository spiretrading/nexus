#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include "spire/spire/resources.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/title_bar.hpp"
#include "spire/ui/window.hpp"

using namespace spire;

class test_window : public QWidget {
  public:
    test_window(QWidget* parent = nullptr)
        : QWidget(parent) {
      setBaseSize(600, 480);
      setStyleSheet("background-color: aqua");
      auto layout = new QHBoxLayout(this);
      auto button = new QPushButton("Button", this);
      layout->addWidget(button);
      window()->setWindowTitle("Test Title");
      connect(button, &QPushButton::clicked, [=, num = 0] () mutable {
        window()->setWindowTitle(window()->windowTitle() +
          QString("%1").arg(++num));
      });
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  window frame(new test_window());
  frame.show();
  application->exec();
}
