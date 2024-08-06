#include "Spire/Login/TrackMenuButton.hpp"
#include <QMovie>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MenuButton.hpp"

using namespace Spire;
using namespace Spire::Styles;

TrackMenuButton::TrackMenuButton(std::vector<Track> tracks,
    std::shared_ptr<TrackModel> current, QWidget* parent)
    : m_current(std::move(current)),
      m_state(State::READY) {
  auto body = new QWidget();
  auto body_layout = make_hbox_layout(body);
  body_layout->addStretch(1);
  auto inner_body_layout = make_vbox_layout();
  inner_body_layout->addStretch(1);
  auto contents_layout = make_hbox_layout();
  m_spinner = new QLabel(this);
  m_spinner->setFixedSize(scale(40, 48));
  auto logo = new QMovie(":/Icons/sign_in/spinner_current.gif", QByteArray());
  logo->setScaledSize(scale(40, 48));
  m_spinner->setMovie(logo);
  m_spinner->movie()->start();
  m_spinner->movie()->jumpToFrame(0);
  m_spinner->movie()->stop();
  contents_layout->addWidget(m_spinner);
  contents_layout->addSpacing(scale_width(8));
  auto wordmark_layout = make_vbox_layout();
  wordmark_layout->addSpacing(scale_height(13));
  auto wordmark = new Icon(
    imageFromSvg(":/Icons/sign_in/wordmark_current.svg", scale(80, 32)));
  set_style(*wordmark, StyleSheet());
  wordmark_layout->addWidget(wordmark);
  wordmark_layout->addStretch(1);
  contents_layout->addLayout(wordmark_layout);
  contents_layout->addSpacing(scale_width(9));
  auto chevron =
    new Icon(imageFromSvg(":/Icons/sign_in/chevron_down.svg", scale(11, 8)));
  set_style(*chevron, StyleSheet());
  contents_layout->addWidget(chevron, 0, Qt::AlignCenter);
  inner_body_layout->addLayout(contents_layout);
  inner_body_layout->addStretch(1);
  body_layout->addLayout(inner_body_layout);
  body_layout->addStretch(1);
  m_button = new MenuButton(*body);
  enclose(*this, *m_button);
}

const std::shared_ptr<TrackModel>& TrackMenuButton::get_current() const {
  return m_current;
}

TrackMenuButton::State TrackMenuButton::get_state() const {
  return m_state;
}

void TrackMenuButton::set_state(State state) {
  if(state == m_state) {
    return;
  }
  m_state = state;
  if(m_state == State::LOADING) {
    m_spinner->movie()->start();
  } else {
    m_spinner->movie()->stop();
    m_spinner->movie()->jumpToFrame(0);
  }
}
