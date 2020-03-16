#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QHBoxLayout>
#include <QContextMenuEvent>
#include <QLabel>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorContextMenu.hpp"

class Widget : public QWidget {
  public:
    Widget() {
      auto layout = new QHBoxLayout(this);
      auto l = new QLabel("Right click to show context menu", this);
      layout->addWidget(l);
    }

  protected:
    void contextMenuEvent(QContextMenuEvent* event) override {
      auto menu = OrderImbalanceIndicatorContextMenu(this);
      menu.exec(event->globalPos());
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = Widget();
  w.resize(600, 300);
  w.show();
  application->exec();
}
