#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include "spire/spire/dimensions.hpp"
#include "spire/spire/resources.hpp"
#include "spire/ui/window.hpp"

using namespace spire;

class test_window : public QWidget {
  public:
    test_window(QWidget* parent = nullptr)
        : QWidget(parent, Qt::FramelessWindowHint |
            Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
            Qt::WindowCloseButtonHint) {
      auto body = new QWidget(this);
      body->setStyleSheet("background-color: blue;");
      body->setFixedSize(scale(400, 400));
      auto window_layout = new QHBoxLayout(this);
      window_layout->setContentsMargins({});
      auto window = new spire::window(body, this);
      window->set_icon(imageFromSvg(":/icons/spire-icon-black.svg", scale(26, 26),
        QRect(translate(8, 8), scale(10, 10))),
        imageFromSvg(":/icons/spire-icon-grey.svg", scale(26, 26),
        QRect(translate(8, 8), scale(10, 10))));
      window_layout->addWidget(window, 1);
      auto layout = new QHBoxLayout(body);
      auto label = new QLabel("test label", body);
      layout->addWidget(label, 1);
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
