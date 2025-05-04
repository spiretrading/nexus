#include "Spire/SignIn/TrackMenuButton.hpp"
#include <QMovie>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/TextBox.hpp"

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

  const QImage& get_logo(Track track) {
    if(track == Track::CLASSIC) {
      static auto icon =
        imageFromSvg(":/Icons/sign_in/logo_classic.svg", scale(32, 32));
      return icon;
    } else if(track == Track::CURRENT) {
      static auto icon =
        imageFromSvg(":/Icons/sign_in/logo_current.svg", scale(32, 32));
      return icon;
    } else if(track == Track::PREVIEW) {
      static auto icon =
        imageFromSvg(":/Icons/sign_in/logo_preview.svg", scale(32, 32));
      return icon;
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
      new Icon(imageFromSvg(get_wordmark_path(track), scale(80, 41)));
    set_style(*wordmark, StyleSheet());
    return wordmark;
  }

  auto make_track_menu_item(
      Track track, std::shared_ptr<BooleanModel> is_checked) {
    static auto checkmark =
      imageFromSvg(":/Icons/sign_in/check.svg", scale(16, 16));
    auto item = new QWidget();
    auto layout = make_hbox_layout(item);
    auto check_icon = new Icon(checkmark);
    auto check_style = StyleSheet();
    check_style.get(Any()).set(Fill(QColor(0x333333)));
    set_style(*check_icon, std::move(check_style));
    if(is_checked->get()) {
      match(*check_icon, Checked());
    }
    update_style(*check_icon, [] (auto& style) {
      style.get(!Checked()).set(Visibility::INVISIBLE);
    });
    is_checked->connect_update_signal([=] (bool is_checked) {
      if(is_checked) {
        match(*check_icon, Checked());
      } else {
        unmatch(*check_icon, Checked());
      }
    });
    layout->addWidget(check_icon, 0, Qt::AlignVCenter);
    layout->addSpacing(scale_width(6));
    auto logo = new Icon(get_logo(track));
    set_style(*logo, StyleSheet());
    layout->addWidget(logo);
    layout->addSpacing(scale_width(8));
    auto text_layout = make_vbox_layout();
    auto name = make_label(to_text(track));
    name->setFixedHeight(scale_height(18));
    update_style(*name, [] (auto& style) {
      style.get(Any()).
        set(FontSize(scale_height(14))).
        set(PaddingBottom(scale_height(2)));
    });
    text_layout->addWidget(name);
    auto description = make_label(get_description(track));
    description->setFixedHeight(scale_height(14));
    update_style(*description, [] (auto& style) {
      style.get(Any()).
        set(FontSize(scale_height(12))).
        set(TextColor(QColor(0x808080)));
    });
    text_layout->addWidget(description);
    layout->addLayout(text_layout);
    return item;
  }
}

TrackMenuButton::TrackMenuButton(std::vector<Track> tracks,
    std::shared_ptr<TrackModel> current, QWidget* parent)
    : m_is_multitrack(tracks.size() > 1),
      m_current(std::move(current)),
      m_state(State::READY),
      m_selected(
        std::make_shared<AssociativeValueModel<Track>>(m_current->get())) {
  auto body = new QWidget();
  auto body_layout = make_hbox_layout(body);
  body_layout->addStretch(1);
  auto inner_body_layout = make_vbox_layout();
  inner_body_layout->addStretch(1);
  auto contents_layout = make_hbox_layout();
  auto spinner_layout = make_vbox_layout();
  m_spinner = make_spinner(m_current->get());
  spinner_layout->addWidget(m_spinner);
  spinner_layout->addStretch(1);
  contents_layout->addLayout(spinner_layout);
  contents_layout->addSpacing(scale_width(8));
  auto wordmark_layout = make_vbox_layout();
  wordmark_layout->addSpacing(scale_height(13));
  m_wordmark = make_wordmark(m_current->get());
  wordmark_layout->addWidget(m_wordmark);
  contents_layout->addLayout(wordmark_layout);
  contents_layout->addSpacing(scale_width(9));
  auto chevron =
    new Icon(imageFromSvg(":/Icons/sign_in/chevron_down.svg", scale(11, 8)));
  if(m_is_multitrack) {
    set_style(*chevron, StyleSheet());
  } else {
    update_style(*chevron, [&] (auto& style) {
      style.get(Any()).set(Visibility::NONE);
    });
  }
  contents_layout->addWidget(chevron, 0, Qt::AlignCenter);
  inner_body_layout->addLayout(contents_layout);
  inner_body_layout->addStretch(1);
  body_layout->addLayout(inner_body_layout);
  body_layout->addStretch(1);
  auto button_box = new Box(body);
  update_style(*button_box, [] (auto& style) {
    style.get(Any()).
      set(border_size(scale_width(1))).
      set(border_color(QColor(Qt::transparent))).
      set(padding(scale_width(8)));
  });
  m_button = new MenuButton(*button_box);
  if(m_is_multitrack) {
    for(auto& track : tracks) {
      m_button->get_menu().add_action(to_text(track),
        std::bind_front(&TrackMenuButton::on_track, this, track),
        make_track_menu_item(track, m_selected->get_association(track)));
    }
  } else {
    m_button->setDisabled(true);
  }
  enclose(*this, *m_button);
  update_style(*m_button, [] (auto& style) {
    style.get(Any() > is_a<ContextMenu>() > is_a<ListView>() >
      is_a<ListItem>()).set(vertical_padding(scale_height(6)));
  });
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
    m_button->setDisabled(true);
    m_spinner->movie()->start();
  } else if(m_state == State::READY) {
    if(m_is_multitrack) {
      m_button->setDisabled(false);
    }
    m_spinner->movie()->stop();
    m_spinner->movie()->jumpToFrame(0);
  } else {
    m_button->setDisabled(true);
    m_spinner->movie()->stop();
    m_spinner->movie()->jumpToFrame(0);
  }
}

void TrackMenuButton::on_current(Track track) {
  m_selected->set(track);
  auto& body_layout =
    *static_cast<Box&>(m_button->get_body()).get_body()->layout();
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
