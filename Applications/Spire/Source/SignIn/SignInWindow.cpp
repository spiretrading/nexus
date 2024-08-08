#include "Spire/SignIn/SignInWindow.hpp"
#include <QKeyEvent>
#include "Spire/SignIn/ChromaHashWidget.hpp"
#include "Spire/SignIn/TrackMenuButton.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(32, 26);
    return size;
  }

  auto BUILD_LABEL_STYLE(StyleSheet style) {
    style.get(Disabled()).set(TextColor(QColor(0xFFFFFF)));
    return style;
  }

  auto CLOSE_BUTTON_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > Body() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(QColor(0xE2E0FF)));
    style.get(!Active() > Body() > is_a<Icon>()).
      set(Fill(QColor(0xBAB3D9)));
    style.get((Hover() || Press()) > Body() > is_a<Icon>()).
      set(BackgroundColor(QColor(0x401D8B))).
      set(Fill(QColor(0xE63F45)));
    return style;
  }

  auto INPUT_STYLE(StyleSheet style) {
    style.get(Any()).
      set(border_size(0)).
      set(FontSize(scale_height(14)));
    return style;
  }

  auto SIGN_IN_BUTTON_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Bold);
    font.setPixelSize(scale_width(14));
    style.get(Any() > Body()).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
      set(text_style(font, QColor(0, 0, 0))).
      set(BackgroundColor(QColor(0x684BC7))).
      set(border(scale_width(1), QColor(0x684BC7))).
      set(TextColor(QColor(0xE2E0FF)));
    style.get(Hover() > Body()).
      set(BackgroundColor(QColor(0x8D78EC))).
      set(border_color(QColor(0x8D78EC)));
    style.get(Focus() > Body()).set(border_color(QColor(0x8D78EC)));
    style.get(Disabled() > Body()).
      set(BackgroundColor(QColor(0x4B23A0))).
      set(TextColor(QColor(0x8D78EC))).
      set(border_color(QColor(0x684BC7)));
    return style;
  }

  auto PASSWORD_INPUT_STYLE(StyleSheet style) {
    style = INPUT_STYLE(std::move(style));
    style.get(Any()).set(EchoMode(QLineEdit::EchoMode::Password));
    return style;
  }

  auto STATUS_LABEL_STYLE(StyleSheet style) {
    style.get(Disabled()).
      set(TextColor(QColor(0xFAEB96))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    return style;
  }
}

SignInWindow::SignInWindow(std::string version, std::vector<Track> tracks,
    std::shared_ptr<TrackModel> track, std::vector<std::string> servers,
    QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_server_box(nullptr),
      m_is_dragging(false),
      m_last_focus(nullptr) {
  setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
  m_shadow = new DropShadow(this);
  setObjectName("SignInWindow");
  setStyleSheet(R"(
    #SignInWindow {
      background-color: #4B23A0;
      border: 1px solid #321471;
    })");
  auto close_button =
    make_icon_button(imageFromSvg(":/Icons/close.svg", BUTTON_SIZE()));
  set_style(*close_button, CLOSE_BUTTON_STYLE());
  close_button->setFixedSize(BUTTON_SIZE());
  close_button->setFocusPolicy(Qt::NoFocus);
  close_button->connect_click_signal([=] { window()->close(); });
  auto layout = make_vbox_layout(this);
  layout->addWidget(close_button, 0, Qt::AlignRight);
  layout->addSpacing(scale_height(22));
  m_track_button = new TrackMenuButton(std::move(tracks), std::move(track));
  m_track_button->setFixedWidth(scale_width(280));
  layout->addWidget(m_track_button, 0, Qt::AlignCenter);
  layout->addSpacing(scale_height(10));
  m_status_label = make_label("");
  update_style(*m_status_label, [&] (auto& style) {
    style = STATUS_LABEL_STYLE(style);
  });
  layout->addWidget(m_status_label, 0, Qt::AlignCenter);
  layout->addSpacing(scale_height(20));
  m_username_text_box = new TextBox();
  m_username_text_box->setFixedSize(scale(280, 30));
  m_username_text_box->get_current()->connect_update_signal(
    [=] (const auto& current) {
      m_sign_in_button->setDisabled(current.isEmpty());
    });
  m_username_text_box->set_placeholder(tr("Username"));
  update_style(*m_username_text_box, [&] (auto& style) {
    style = INPUT_STYLE(style);
  });
  m_username_key_observer.emplace(*m_username_text_box);
  m_username_key_observer->connect_key_press_signal(
    std::bind_front(&SignInWindow::on_key_press, this));
  layout->addWidget(m_username_text_box, 0, Qt::AlignCenter);
  layout->addSpacing(scale_height(15));
  auto password_layout = make_hbox_layout();
  password_layout->setContentsMargins(scale_width(52), 0, scale_width(52), 0);
  m_password_text_box = new TextBox();
  m_password_text_box->set_placeholder(tr("Password"));
  update_style(*m_password_text_box, [&] (auto& style) {
    style = PASSWORD_INPUT_STYLE(style);
  });
  m_password_text_box->get_current()->connect_update_signal(
    [=] (const auto& current) {
      m_chroma_hash_widget->set_text(current);
    });
  m_password_key_observer.emplace(*m_password_text_box);
  m_password_key_observer->connect_key_press_signal(
    std::bind_front(&SignInWindow::on_key_press, this));
  password_layout->addWidget(m_password_text_box);
  m_chroma_hash_widget = new ChromaHashWidget();
  m_chroma_hash_widget->setFixedWidth(scale_width(34));
  m_chroma_hash_widget->setContentsMargins(
    {scale_width(2), scale_height(2), scale_width(2), scale_height(2)});
  password_layout->addWidget(m_chroma_hash_widget);
  layout->addLayout(password_layout);
  if(servers.size() > 1) {
    layout->addSpacing(scale_height(15));
    auto server_list =
      std::make_shared<ArrayListModel<std::string>>(std::move(servers));
    m_server_box = new DropDownBox(server_list);
    m_server_box->setFixedSize(scale(280, 30));
    m_server_box->get_current()->set(0);
    update_style(*m_server_box, [&] (auto& style) {
      style.get((Hover() || FocusIn()) > is_a<TextBox>()).
        set(border_color(QColor(0xFFA95E)));
      style.get(Any() > is_a<DropDownList>() >
          is_a<ListView>() > is_a<ListItem>() > Body()).set(
        FontSize(scale_height(14)));
      style.get(Any() > is_a<DropDownList>() >
          is_a<ListView>() > is_a<ListItem>()).set(
        vertical_padding(scale_height(7)));
      style.get(Any() > is_a<TextBox>()).set(
        FontSize(scale_height(14)));
    });
    layout->addWidget(m_server_box, 0, Qt::AlignCenter);
    setFixedSize(scale(384, 391));
  } else {
    setFixedSize(scale(384, 346));
  }
  layout->addSpacing(scale_height(30));
  auto button_layout = make_hbox_layout();
  button_layout->setContentsMargins(scale_width(52), 0, scale_width(52), 0);
  auto build_label =
    make_label(QString(tr("Build ")) + QString::fromStdString(version));
  update_style(*build_label, [&] (auto& style) {
    style = BUILD_LABEL_STYLE(style);
  });
  button_layout->addWidget(build_label);
  button_layout->addStretch(103);
  m_sign_in_button = make_label_button(tr("Sign In"));
  m_sign_in_button->setFixedSize(scale(120, 30));
  set_style(*m_sign_in_button, SIGN_IN_BUTTON_STYLE());
  m_sign_in_button->connect_click_signal(
    std::bind_front(&SignInWindow::try_sign_in, this));
  m_sign_in_button->setDisabled(true);
  button_layout->addWidget(m_sign_in_button);
  layout->addLayout(button_layout);
  layout->addSpacing(scale_height(48));
  setTabOrder(m_username_text_box, m_password_text_box);
  if(m_server_box) {
    setTabOrder(m_password_text_box, m_server_box);
    setTabOrder(m_server_box, m_sign_in_button);
  } else {
    setTabOrder(m_password_text_box, m_sign_in_button);
  }
  set_state(State::NONE);
}

void SignInWindow::set_state(State state) {
  switch(state) {
    case State::NONE: {
      reset_all();
      break;
    }
    case State::SIGNING_IN: {
      m_username_text_box->setEnabled(false);
      m_password_text_box->setEnabled(false);
      if(m_server_box) {
        m_server_box->setEnabled(false);
      }
      static_cast<TextBox&>(
        m_sign_in_button->get_body()).get_current()->set(tr("Cancel"));
      m_status_label->get_current()->set("");
      m_track_button->set_state(TrackMenuButton::State::LOADING);
      break;
    }
    case State::CANCELLING: {
      reset_all();
      state = State::NONE;
      break;
    }
    case State::ERROR: {
      m_status_label->get_current()->set(tr("Sign in failed."));
      reset_visuals();
      break;
    }
  }
  m_state = state;
}

void SignInWindow::set_error(const QString& message) {
  m_status_label->get_current()->set(message);
  reset_visuals();
  m_state = State::ERROR;
}

connection SignInWindow::connect_sign_in_signal(
    const SignInSignal::slot_type& slot) const {
  return m_sign_in_signal.connect(slot);
}

connection SignInWindow::connect_cancel_signal(
    const CancelSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

void SignInWindow::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    window()->close();
  } else if(m_password_text_box->hasFocus() || m_server_box &&
      find_focus_state(*m_server_box) != FocusObserver::State::NONE) {
    return;
  } else if(!m_username_text_box->hasFocus() &&
      m_username_text_box->get_current()->get().isEmpty()) {
    m_username_text_box->get_current()->set(event->text());
    m_username_text_box->setFocus();
  }
}

void SignInWindow::mouseMoveEvent(QMouseEvent* event) {
  if(!m_is_dragging) {
    return;
  }
  auto delta = event->globalPos();
  delta -= m_last_pos;
  auto window_pos = window()->pos();
  window_pos += delta;
  m_last_pos = event->globalPos();
  window()->move(window_pos);
}

void SignInWindow::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_pos = event->globalPos();
}

void SignInWindow::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void SignInWindow::reset_all() {
  m_status_label->get_current()->set("");
  reset_visuals();
}

void SignInWindow::reset_visuals() {
  m_username_text_box->setEnabled(true);
  m_password_text_box->setEnabled(true);
  if(m_server_box) {
    m_server_box->setEnabled(true);
  }
  if(m_last_focus) {
    m_last_focus->setFocus();
    m_last_focus = nullptr;
  } else {
    m_sign_in_button->setFocus();
  }
  static_cast<TextBox&>(
    m_sign_in_button->get_body()).get_current()->set(tr("Sign In"));
  m_track_button->set_state(TrackMenuButton::State::READY);
}

void SignInWindow::try_sign_in() {
  if(!m_sign_in_button->isEnabled()) {
    return;
  }
  if(m_state != State::SIGNING_IN) {
    if(m_username_text_box->get_current()->get().isEmpty()) {
      set_error(tr("Incorrect username or password."));
    } else {
      auto server = [&] {
        if(m_server_box) {
          if(auto current = m_server_box->get_current()->get()) {
            auto& servers = static_cast<ListModel<std::string>&>(
              *m_server_box->get_list().get());
            return servers.get(*current);
          }
        }
        return std::string();
      }();
      m_sign_in_signal(m_username_text_box->get_current()->get().toStdString(),
        m_password_text_box->get_current()->get().toStdString(),
        m_track_button->get_current()->get(), server);
      set_state(State::SIGNING_IN);
    }
  } else {
    m_cancel_signal();
    set_state(State::CANCELLING);
  }
}

void SignInWindow::on_key_press(QWidget& target, const QKeyEvent& event) {
  if(event.key() == Qt::Key_Enter || event.key() == Qt::Key_Return) {
    if(m_state == State::NONE || m_state == State::ERROR) {
      m_last_focus = &target;
      try_sign_in();
      m_sign_in_button->setFocus();
    }
  }
}
