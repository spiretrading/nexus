#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Styles/CubicBezierExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

struct ActivityMessageBox : QWidget {
  enum class State {
    NONE,
    DOWNLOADING,
    DOWNLOAD_COMPLETE,
    INSTALLING
  };
  using StateStyle = StateSelector<State, struct StateStyleTag>;
  using Label = StateSelector<void, struct LabelStateTag>;
  TextBox* m_label;
  State m_state;

  ActivityMessageBox()
      : m_state(State::NONE) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto layout = make_vbox_layout(this);
    m_label = make_label(QString());
    match(*m_label, Label());
    layout->addWidget(m_label);
    m_label->get_current()->set(tr("Update available."));
    update_style(*this, [] (auto& style) {
      style.get(Any() > Label()).
        set(PaddingBottom(scale_height(6))).
        set(PaddingLeft(scale_width(76))).
        set(TextColor(QColor(0xFFFFFF)));
      style.get(StateStyle(State::DOWNLOADING) > Label()).
        set(PaddingLeft(ease(scale_width(76), 0, milliseconds(800))));
    });
  }

  void set_state(State state) {
    if(state == m_state) {
      return;
    }
    if(state == State::DOWNLOADING) {
      m_label->get_current()->set(tr("Downloading update..."));
      match(*this, StateStyle(State::DOWNLOADING));
    }
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
  QTimer::singleShot(1000, [&] {
    window.m_activity_message_box->set_state(
      ActivityMessageBox::State::DOWNLOADING);
  });
  application.exec();
}
