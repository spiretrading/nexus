#include <QApplication>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

struct ActivityMessageBox : QWidget {
  TextBox* m_label;

  enum class State {
    NONE,
    ACTIVE,
    DOWNLOADING,
    DOWNLOAD_COMPLETE,
    INSTALLING
  };

  ActivityMessageBox() {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto layout = make_vbox_layout(this);
    m_label = make_label(QString());
    layout->addWidget(m_label);
    m_label->get_current()->set(tr("Update available."));
    update_style(*m_label, [] (auto& style) {
      style.get(Any()).
        set(PaddingBottom(scale_height(6))).
        set(TextColor(QColor(0xFFFFFF))).
        set(TextAlign(Qt::AlignCenter));
    });
  }
};

struct UpdaterBox : QWidget {
  ActivityMessageBox* m_activity_message_box;

  UpdaterBox() {
    auto body = new QWidget();
    auto layout = make_vbox_layout(body);
    layout->addSpacing(scale_height(38));
    m_activity_message_box = new ActivityMessageBox();
    layout->addWidget(m_activity_message_box);
    auto box = new Box(body);
    enclose(*this, *box);
    proxy_style(*this, *box);
    update_style(*this, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0x4B23A0)));
    });
    setFixedSize(scale(280, 232));
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = UpdaterBox();
  window.show();
  application.exec();
}
