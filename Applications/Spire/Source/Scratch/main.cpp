#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/ListItem.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Hover());
    return style;
  }

  struct Dummy : QWidget {
    QWidget* m_body;

    void mount(QWidget& body) {
      m_body = &body;
      m_body->setParent(this);
      m_body->show();
      set_style(body, DEFAULT_STYLE());
    }

    void unmount() {
      delete m_body;
    }
  };
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto item = Dummy();
  item.show();
  auto timer = QTimer();
  timer.setInterval(1);
  auto i = 0;
  timer.connect(&timer, &QTimer::timeout, [&] {
    if(i % 2 == 0) {
      item.mount(*new QWidget());
    } else {
      item.unmount();
    }
    ++i;
  });
  timer.start();
  application.exec();
}
