#include "Spire/Login/LoginWindow.hpp"
#include <QMovie>
#include "Spire/Login/ChromaHashWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DropShadow.hpp"
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
    style.get(Disabled()).set(TextColor(QColor(0xFF, 0xFF, 0xFF)));
    return style;
  }

  auto CLOSE_BUTTON_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(QColor(0xE2E0FF)));
    style.get(!Active() > is_a<Icon>()).
      set(Fill(QColor(0xBAB3D9)));
    style.get((Hover() || Press()) > is_a<Icon>()).
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
      set(TextColor(QColor(0xFA, 0xEB, 0x96))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    return style;
  }
}

LoginWindow::LoginWindow(const std::string& version, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_is_dragging(false) {
  setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
  setFixedSize(scale(384, 346));
  m_shadow = new DropShadow(this);
  setObjectName("LoginWindow");
  setStyleSheet(R"(
    #LoginWindow {
      background-color: #4B23A0;
      border: 1px solid #321471;
    })");
  auto close_button =
    make_icon_button(imageFromSvg(":/Icons/close.svg", BUTTON_SIZE()), this);
  set_style(*close_button, CLOSE_BUTTON_STYLE());
  close_button->setFixedSize(BUTTON_SIZE());
  close_button->setFocusPolicy(Qt::NoFocus);
  close_button->connect_clicked_signal([=] { window()->close(); });
  auto layout = make_vbox_layout(this);
  layout->addWidget(close_button, 0, Qt::AlignRight);
  layout->addSpacing(scale_height(30));
  m_logo_widget = new QLabel(parent);
  m_logo_widget->setFixedSize(scale(134, 50));
  auto logo = new QMovie(":/Icons/logo.gif", QByteArray(), this);
  logo->setScaledSize(scale(134, 50));
  m_logo_widget->setMovie(logo);
  logo->start();
  layout->addWidget(m_logo_widget, 0, Qt::AlignCenter);
  layout->addSpacing(scale_height(23));
  m_status_label = new TextBox(this);
  m_status_label->set_read_only(true);
  m_status_label->setDisabled(true);
  update_style(*m_status_label, [&] (auto& style) {
    style = STATUS_LABEL_STYLE(style);
  });
  layout->addWidget(m_status_label, 0, Qt::AlignCenter);
  layout->addSpacing(scale_height(20));
  m_username_text_box = new TextBox(this);
  m_username_text_box->setFixedSize(scale(280, 30));
  m_username_text_box->get_current()->connect_update_signal(
    [=] (const auto& current) {
      m_sign_in_button->setDisabled(current.isEmpty());
    });
  m_username_text_box->set_placeholder(tr("Username"));
  update_style(*m_username_text_box, [&] (auto& style) {
    style = INPUT_STYLE(style);
  });
  layout->addWidget(m_username_text_box, 0, Qt::AlignCenter);
  layout->addSpacing(scale_height(15));
  auto password_layout = make_hbox_layout();
  password_layout->setContentsMargins(scale_width(52), 0, scale_width(52), 0);
  m_password_text_box = new TextBox(this);
  m_password_text_box->set_placeholder(tr("Password"));
  update_style(*m_password_text_box, [&] (auto& style) {
    style = PASSWORD_INPUT_STYLE(style);
  });
  m_password_text_box->get_current()->connect_update_signal(
    [=] (const auto& current) {
      m_chroma_hash_widget->set_text(current);
    });
  password_layout->addWidget(m_password_text_box);
  m_chroma_hash_widget = new ChromaHashWidget(this);
  m_chroma_hash_widget->setFixedSize(scale(34, 30));
  m_chroma_hash_widget->setContentsMargins(
    {scale_width(2), scale_height(2), scale_width(2), scale_height(2)});
  password_layout->addWidget(m_chroma_hash_widget);
  layout->addLayout(password_layout);
  layout->addSpacing(scale_height(30));
  auto button_layout = make_hbox_layout();
  button_layout->setContentsMargins(scale_width(52), 0, scale_width(52), 0);
  auto build_label =
    new TextBox(QString(tr("Build ")) + QString::fromStdString(version), this);
  build_label->set_read_only(true);
  build_label->setDisabled(true);
  update_style(*build_label, [&] (auto& style) {
    style = BUILD_LABEL_STYLE(style);
  });
  button_layout->addWidget(build_label);
  button_layout->addStretch(103);
  m_sign_in_button = make_label_button(tr("Sign In"), this);
  m_sign_in_button->setFixedSize(scale(120, 30));
  set_style(*m_sign_in_button, SIGN_IN_BUTTON_STYLE());
  m_sign_in_button->connect_clicked_signal([=] { try_login(); });
  m_sign_in_button->setDisabled(true);
  button_layout->addWidget(m_sign_in_button);
  layout->addLayout(button_layout);
  layout->addSpacing(scale_height(48));
  setTabOrder(m_username_text_box, m_password_text_box);
  setTabOrder(m_password_text_box, m_sign_in_button);
  set_state(State::NONE);
}

LoginWindow::~LoginWindow() = default;

void LoginWindow::set_state(State state) {
  switch(state) {
    case State::NONE: {
      reset_all();
      break;
    }
    case State::LOGGING_IN: {
      m_username_text_box->setEnabled(false);
      m_password_text_box->setEnabled(false);
      static_cast<TextBox&>(
        m_sign_in_button->get_body()).get_current()->set(tr("Cancel"));
      m_status_label->get_current()->set("");
      m_logo_widget->movie()->start();
      break;
    }
    case State::CANCELLING: {
      reset_all();
      state = State::NONE;
      break;
    }
    case State::INCORRECT_CREDENTIALS: {
      m_status_label->get_current()->set(
        tr("Incorrect username or password."));
      reset_visuals();
      break;
    }
    case State::SERVER_UNAVAILABLE: {
      m_status_label->get_current()->set(tr("Server is unavailable."));
      reset_visuals();
      break;
    }
  }
  m_state = state;
}

connection LoginWindow::connect_login_signal(
    const LoginSignal::slot_type& slot) const {
  return m_login_signal.connect(slot);
}

connection LoginWindow::connect_cancel_signal(
    const CancelSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

void LoginWindow::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    window()->close();
  } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(m_password_text_box->hasFocus()) {
      if(!m_username_text_box->get_current()->get().isEmpty()) {
        try_login();
      }
    }
  } else if(m_password_text_box->hasFocus()) {
    return;
  } else if(!m_username_text_box->hasFocus() &&
      m_username_text_box->get_current()->get().isEmpty()) {
    m_username_text_box->get_current()->set(event->text());
    m_username_text_box->setFocus();
  }
}

void LoginWindow::mouseMoveEvent(QMouseEvent* event) {
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

void LoginWindow::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_pos = event->globalPos();
}

void LoginWindow::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void LoginWindow::reset_all() {
  m_status_label->get_current()->set("");
  reset_visuals();
}

void LoginWindow::reset_visuals() {
  m_username_text_box->setEnabled(true);
  m_password_text_box->setEnabled(true);
  m_sign_in_button->setFocus();
  static_cast<TextBox&>(
    m_sign_in_button->get_body()).get_current()->set(tr("Sign In"));
  m_logo_widget->movie()->stop();
  m_logo_widget->movie()->jumpToFrame(0);
}

void LoginWindow::try_login() {
  if(m_state != State::LOGGING_IN) {
    if(m_username_text_box->get_current()->get().isEmpty()) {
      set_state(State::INCORRECT_CREDENTIALS);
    } else {
      m_login_signal(
        m_username_text_box->get_current()->get().toStdString(),
        m_password_text_box->get_current()->get().toStdString());
      set_state(State::LOGGING_IN);
    }
  } else {
    m_cancel_signal();
    set_state(State::CANCELLING);
  }
}
