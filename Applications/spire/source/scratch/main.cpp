#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include "spire/spire/resources.hpp"
#include "spire/time_and_sales/loading_widget.hpp"
#include "spire/ui/window.hpp"

using namespace spire;

class test_window : public QWidget {
  public:
    test_window() {
      auto body = new QWidget(this);
      body->setMinimumSize(50, 60);
      resize(400, 650);
      auto window_layout = new QVBoxLayout(this);
      window_layout->setContentsMargins({});
      auto w = new spire::window(body, this);
      setWindowTitle("Loading Widget Test");
      window_layout->addWidget(w);
      auto layout = new QVBoxLayout(body);
      layout->setContentsMargins({});
      auto loading = new loading_widget(this);
      layout->addWidget(loading);
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new test_window();
  w->show();
  application->exec();
}
