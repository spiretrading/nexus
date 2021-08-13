#include "Spire/Login/LoginWindow.hpp"
#include <QHBoxLayout>
#include <QMovie>
#include <QVBoxLayout>
#include "Spire/Login/ChromaHashWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/Icon.hpp"
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
    style.get(Body()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(Fill(QColor::fromRgb(0xE2, 0xE0, 0xFF)));
    style.get(!Active() / Body()).
      set(Fill(QColor::fromRgb(0xBA, 0xB3, 0xD9)));
    style.get(Hover() / Body()).
      set(BackgroundColor(QColor::fromRgb(0x32, 0x14, 0x71))).
      set(Fill(QColor::fromRgb(0xE6, 0x3F, 0x45)));
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
    style.get(Body()).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
      set(text_style(font, QColor(0, 0, 0))).
      set(BackgroundColor(QColor(0x68, 0x4B, 0xC7))).
      set(border(scale_width(1), QColor(0x68, 0x4B, 0xC7))).
      set(TextColor(QColor(0xE2, 0xE0, 0xFF)));
    style.get(Hover() / Body()).
      set(BackgroundColor(QColor(0x8D, 0x78, 0xEC))).
      set(border_color(QColor(0x8D, 0x78, 0xEC)));
    style.get(Focus() / Body()).
      set(border_color(QColor(0x8D, 0x78, 0xEC)));
    style.get(Disabled() / Body()).
      set(BackgroundColor(QColor(0x4B, 0x23, 0xA0))).
      set(TextColor(QColor(0x8D, 0x78, 0xEC))).
      set(border_color(QColor(0x68, 0x4B, 0xC7)));
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
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_close_button =
    make_icon_button(imageFromSvg(":/Icons/close.svg", BUTTON_SIZE()), this);
  set_style(*m_close_button, CLOSE_BUTTON_STYLE());
  m_close_button->setFixedSize(BUTTON_SIZE());
  m_close_button->setFocusPolicy(Qt::NoFocus);
  m_close_button->connect_clicked_signal([=] { window()->close(); });
  layout->addWidget(m_close_button, 0, Qt::AlignRight);
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
  set_style(*m_status_label, STATUS_LABEL_STYLE(get_style(*m_status_label)));
  layout->addWidget(m_status_label, 0, Qt::AlignCenter);
  layout->addSpacing(scale_height(20));
  m_username_text_box = new TextBox(this);
  m_username_text_box->setFixedSize(scale(280, 30));
  m_username_text_box->get_model()->connect_current_signal(
    [=] (const auto& current) {
      m_sign_in_button->setDisabled(current.isEmpty());
    });
  m_username_text_box->set_placeholder(tr("Username"));
  set_style(*m_username_text_box, INPUT_STYLE(get_style(*m_username_text_box)));
  layout->addWidget(m_username_text_box, 0, Qt::AlignCenter);
  layout->addSpacing(scale_height(15));
  auto password_layout = new QHBoxLayout();
  password_layout->setContentsMargins(scale_width(52), 0, scale_width(52), 0);
  password_layout->setSpacing(0);
  m_password_text_box = new TextBox(this);
  m_password_text_box->set_placeholder(tr("Password"));
  set_style(*m_password_text_box, PASSWORD_INPUT_STYLE(get_style(
    *m_password_text_box)));
  m_password_text_box->get_model()->connect_current_signal(
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
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(scale_width(52), 0, scale_width(52), 0);
  button_layout->setSpacing(0);
  auto build_label =
    new TextBox(QString(tr("Build ")) + QString::fromStdString(version), this);
  build_label->set_read_only(true);
  build_label->setDisabled(true);
  set_style(*build_label, BUILD_LABEL_STYLE(get_style(*build_label)));
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
      static_cast<TextBox&>(m_sign_in_button->get_body()).get_model()->
        set_current(tr("Cancel"));
      m_status_label->get_model()->set_current("");
      m_logo_widget->movie()->start();
      break;
    }
    case State::CANCELLING: {
      reset_all();
      state = State::NONE;
      break;
    }
    case State::INCORRECT_CREDENTIALS: {
      m_status_label->get_model()->set_current(
        tr("Incorrect username or password."));
      reset_visuals();
      break;
    }
    case State::SERVER_UNAVAILABLE: {
      m_status_label->get_model()->set_current(tr("Server is unavailable."));
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
      if(!m_username_text_box->get_model()->get_current().isEmpty()) {
        try_login();
      }
    }
  } else if(m_password_text_box->hasFocus()) {
    return;
  } else if(!m_username_text_box->hasFocus() &&
      m_username_text_box->get_model()->get_current().isEmpty()) {
    m_username_text_box->get_model()->set_current(event->text());
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
  m_status_label->get_model()->set_current("");
  reset_visuals();
}

void LoginWindow::reset_visuals() {
  m_username_text_box->setEnabled(true);
  m_password_text_box->setEnabled(true);
  m_sign_in_button->setFocus();
  static_cast<TextBox&>(m_sign_in_button->get_body()).get_model()->set_current(
    tr("Sign In"));
  m_logo_widget->movie()->stop();
  m_logo_widget->movie()->jumpToFrame(0);
}

void LoginWindow::try_login() {
  if(m_state != State::LOGGING_IN) {
    if(m_username_text_box->get_model()->get_current().isEmpty()) {
      set_state(State::INCORRECT_CREDENTIALS);
    } else {
      m_login_signal(
        m_username_text_box->get_model()->get_current().toStdString(),
        m_password_text_box->get_model()->get_current().toStdString());
      set_state(State::LOGGING_IN);
    }
  } else {
    m_cancel_signal();
    set_state(State::CANCELLING);
  }
}
