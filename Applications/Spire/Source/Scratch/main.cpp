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
      style.get(StateStyle(State::DOWNLOAD_COMPLETE) > Label()).
        set(PaddingLeft(0));
      style.get(StateStyle(State::INSTALLING) > Label()).
        set(PaddingLeft(0));
    });
  }

  void set_state(State state) {
    if(state == m_state) {
      return;
    }
    unmatch(*this, StateStyle(m_state));
    m_state = state;
    if(state == State::DOWNLOADING) {
      m_label->get_current()->set(tr("Downloading update..."));
    } else if(state == State::DOWNLOAD_COMPLETE) {
      m_label->get_current()->set(tr("Download complete"));
    } else if(state == State::INSTALLING) {
      m_label->get_current()->set(tr("Installing update..."));
    }
    match(*this, StateStyle(m_state));
  }
};

struct ProgressBox : QWidget {
  ProgressBox() {
    auto box = new Box();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    enclose(*this, *box);
    proxy_style(*this, *box);
    update_style(*this, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0x321471)));
    });
  }

  QSize sizeHint() const {
    return scale(1, 4);
  }
};

struct TimeLeftBox : QWidget {
  TextBox* m_label;

  TimeLeftBox() {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto layout = make_vbox_layout(this);
    m_label = make_label(tr("About 5s left"));
    layout->addWidget(m_label);
    update_style(*m_label, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(10));
      style.get(Any()).
        set(text_style(font, QColor(0xE2E0FF))).
        set(PaddingTop(scale_height(4)));
    });
  }
};

struct UpdaterBox : QWidget {
  ActivityMessageBox* m_activity_message_box;
  ProgressBox* m_progress_box;
  TimeLeftBox* m_time_left_box;

  UpdaterBox() {
    auto body = new QWidget();
    auto layout = make_vbox_layout(body);
    layout->addSpacing(scale_height(38));
    m_activity_message_box = new ActivityMessageBox();
    layout->addWidget(m_activity_message_box);
    m_progress_box = new ProgressBox();
    m_progress_box->setSizePolicy(
      QSizePolicy::Expanding, m_progress_box->sizePolicy().verticalPolicy());
    layout->addWidget(m_progress_box);
    m_time_left_box = new TimeLeftBox();
    layout->addWidget(m_time_left_box);
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
    QTimer::singleShot(2000, [&] {
      window.m_activity_message_box->set_state(
        ActivityMessageBox::State::DOWNLOAD_COMPLETE);
      QTimer::singleShot(2000, [&] {
        window.m_activity_message_box->set_state(
          ActivityMessageBox::State::INSTALLING);
      });
    });
  });
  application.exec();
}
