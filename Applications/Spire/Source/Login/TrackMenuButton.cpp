#include "Spire/Login/TrackMenuButton.hpp"
#include <QMovie>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MenuButton.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  const QString& get_spinner_path(Track track) {
    if(track == Track::CLASSIC) {
      static auto path = QString(":/Icons/sign_in/spinner_classic.gif");
      return path;
    } else if(track == Track::CURRENT) {
      static auto path = QString(":/Icons/sign_in/spinner_current.gif");
      return path;
    } else if(track == Track::PREVIEW) {
      static auto path = QString(":/Icons/sign_in/spinner_preview.gif");
      return path;
    }
    throw std::runtime_error("Unknown track.");
  }

  const QString& get_wordmark_path(Track track) {
    if(track == Track::CLASSIC) {
      static auto path = QString(":/Icons/sign_in/wordmark_classic.svg");
      return path;
    } else if(track == Track::CURRENT) {
      static auto path = QString(":/Icons/sign_in/wordmark_current.svg");
      return path;
    } else if(track == Track::PREVIEW) {
      static auto path = QString(":/Icons/sign_in/wordmark_preview.svg");
      return path;
    }
    throw std::runtime_error("Unknown track.");
  }

  auto make_spinner(Track track) {
    auto spinner = new QLabel();
    spinner->setFixedSize(scale(40, 48));
    auto logo = new QMovie(get_spinner_path(track), QByteArray());
    logo->setScaledSize(scale(40, 48));
    spinner->setMovie(logo);
    spinner->movie()->start();
    spinner->movie()->jumpToFrame(0);
    spinner->movie()->stop();
    return spinner;
  }

  auto make_wordmark(Track track) {
    auto wordmark =
      new Icon(imageFromSvg(get_wordmark_path(track), scale(80, 32)));
    set_style(*wordmark, StyleSheet());
    return wordmark;
  }
}

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
  m_spinner = make_spinner(m_current->get());
  contents_layout->addWidget(m_spinner);
  contents_layout->addSpacing(scale_width(8));
  auto wordmark_layout = make_vbox_layout();
  wordmark_layout->addSpacing(scale_height(13));
  m_wordmark = make_wordmark(m_current->get());
  wordmark_layout->addWidget(m_wordmark);
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
  for(auto& track : tracks) {
    m_button->get_menu().add_action(
      to_text(track), std::bind_front(&TrackMenuButton::on_track, this, track));
  }
  enclose(*this, *m_button);
  m_connection = m_current->connect_update_signal(
    std::bind_front(&TrackMenuButton::on_current, this));
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

void TrackMenuButton::on_current(Track track) {
  auto& body_layout = *m_button->get_body().layout();
  auto spinner = make_spinner(track);
  auto item = body_layout.replaceWidget(m_spinner, spinner);
  delete item;
  delete m_spinner;
  m_spinner = spinner;
  auto wordmark = make_wordmark(track);
  item = body_layout.replaceWidget(m_wordmark, wordmark);
  delete item;
  delete m_wordmark;
  m_wordmark = wordmark;
}

void TrackMenuButton::on_track(Track track) {
  if(track != m_current->get()) {
    m_current->set(track);
  }
}
